#include <bits/stdc++.h>
#include <pthread.h>
#include <Windows.h>
#include <mutex>
using namespace std;
/*
内存里有两个版本：
	原版数据：只读、MVCC回写 
	MVCC副本：读写
关键数据：
	数据文件：每一段时间落盘一次，从原始数据开始执行并清空redo log
	undo log：transaction过程中记录反向操作，如果abort就执行一遍反向操作，不需要落盘，存在内存里即可
		可以直接用MVCC副本和原数据来实现，不需要记录undo log
	redo log：落盘间期的任何操作
多程序并行视作多机器，用redo log实现最终一致性
数据文件锁：落盘时锁住 
redo log锁：把操作记录到文件时锁住 
	
支持插入、删除、查找
    表达式解析器
支持ACID
    Atomicity原子性
        undo log:
            对于PUT操作，记录对应的DEL操作
            对于DEL操作，记录对应的PUT操作
        transaction开始的时候就创一个undo log
        收到abort的时候就走一遍undo log
    Consistency一致性
        数据库的状态和执?的预期结果?致
        consistent state before and after transactions
        设定几个原则：
            实体完整性：key唯一
            列完整性：value size不超过65535(64KB)
    Isolation隔离性
        根据不同的隔离级别，实现事务之间的互不?扰，需要并发控制算法
        并发控制算法MVCC:
            每次transaction，判断是只读还是读写，
            如果是读写，先检查copy权限锁，没锁的话copy一份数据，在copy的数据上做操作，然后锁住原来的，替换掉
            锁了的话，等copy解锁再做
            如果是只读，在原版本读就行了
    Durability持久化
        redo log
            对于任何操作，记录到log内
            COMMIT时，不着急落盘
            设置一个间隔时间，比如10秒，每10秒落一次盘，清空redo log
            启动的时候再check一下有没有没做的redo log
*/
/*
支持的命令：
    PUT A 3
    GET A
    DEL A
    PUT C (C+1)
    BEGIN
    COMMIT
    BEGIN
    ABORT
协议：
	uid command
	uid DISCONNECT
*/
unordered_map<string,string> data;// 原始数据 
unordered_map<string,string> mvcc_data;// MVCC副本 
unordered_map<string,queue<string>> cmds;// 每个客户端的请求指令 
set<string>clientSet;// 客户端集合 
int timeToDisk=10;// 落盘间隔 
mutex lockData;
mutex lockMvcc;
mutex lockRedo;
mutex lockDisk;
mutex lockCmds;
mutex lockClientSet;
ofstream ofsRedo;
bool consistency_unique_key(){
	// useless, unordered_map has unique key
	return true;
}
bool consistency_value_size(){
	for(unordered_map<string,string>::iterator i=data.begin();i!=data.end();i++)
		if((i->second).size()>=65535)
			return false;
	return true;
}
void writeRedoLog(string cmd){
	lockRedo.lock();
	ofsRedo<<cmd<<endl;
	ofsRedo.flush();
	lockRedo.unlock();
}
void clearRedoLog(){
	lockRedo.lock();
	ofsRedo.close();
	ofsRedo.open("redo.log");
	ofsRedo.close();
	ofsRedo.open("redo.log",ios::app);
	lockRedo.unlock();
}
void* toDisk(void* args){
	// data落盘，清空redo log
	while(1){
		Sleep(timeToDisk*1000);
		lockData.lock();
		// lockDisk.lock();
		ofstream ofs("data.kvdb");
		for(unordered_map<string,string>::iterator i=data.begin();i!=data.end();i++)
			ofs<<i->first<<" "<<i->second<<endl;
		ofs.flush();
		ofs.close();
		clearRedoLog();
		// lockDisk.unlock();
		lockData.unlock();
	}
   return 0;
}
string calcFormularMvcc(string formular){
	stringstream ss;
	int a,b;
	if(formular[0]=='('){
		for(int i=1;i<formular.size();i++){
			if(formular[i]=='+'||formular[i]=='-'){
				string key=formular.substr(1,i-1);
				ss.clear();
				ss<<mvcc_data[key];
				ss>>a;
				string off=formular.substr(i+1,formular.size()-i-2);
				ss.clear();
				ss<<off;
				ss>>b;
				ss.clear();
				if(formular[i]=='+') ss<<a+b;
				else ss<<a-b;
				ss>>key;
				return key;
			}
		}
	}else return formular;
}
string calcFormularData(string formular){
	stringstream ss;
	int a,b;
	if(formular[0]=='('){
		for(int i=1;i<formular.size();i++){
			if(formular[i]=='+'||formular[i]=='-'){
				string key=formular.substr(1,i-1);
				ss.clear();
				ss<<data[key];
				ss>>a;
				string off=formular.substr(i+1,formular.size()-i-2);
				ss.clear();
				ss<<off;
				ss>>b;
				ss.clear();
				if(formular[i]=='+') ss<<a+b;
				else ss<<a-b;
				ss>>key;
				return key;
			}
		}
	}else return formular;
}
void* processThread(void* args){
	string clientId=(char*)args;
	stringstream ss;
	// queue<string>undoLog;
	queue<string>redoLogTrans;// 当前transaction的redo log
	bool transactionBegin=false;// 是否开起了一个transactoin 
	bool transactionReadonly=true;// 当前Transaction是否一直在read 
	/*
	command format:
		clientId PUT A B
		clientId DISCONNECT
		clientId BEGIN
	*/
	while(1){
		while(!cmds[clientId].empty()){
			ss.clear();
			string cmd=cmds[clientId].front();
			ss<<cmd;
			string type,key;
			ss>>type;
			if(type=="DISCONNECT"){
				clientSet.erase(clientSet.find(clientId));
				cout<<clientId<<" disconnected"<<endl;
				return 0;
			}else if(type=="BEGIN"){
				// 开始记录undo log
				if(!transactionBegin){
					transactionBegin=true;
				}
				lockMvcc.lock();
				cout<<clientId<<" begin a transaction"<<endl;
			}else if(type=="COMMIT"){
				// 删掉undo log，副本原子地写回原本
				lockData.lock();
				data=mvcc_data;
				lockData.unlock();
				lockMvcc.unlock();
				//undoLog.clear();
				while(!redoLogTrans.empty()){
					string redoCmd=redoLogTrans.front();
					redoLogTrans.pop();
					writeRedoLog(redoCmd);
				}
				transactionReadonly=true;
				transactionBegin=false;
				cout<<clientId<<" commit a transaction"<<endl;
			}else if(type=="ABORT"){
				// lockData.lock();
				mvcc_data=data;
				lockMvcc.unlock();
				// lockData.unlock();
				transactionReadonly=true;
				transactionBegin=false;
				cout<<clientId<<" abort a transaction"<<endl;
			}else{
				ss>>key;
				if(type=="GET"){
					if(transactionReadonly){
						if(data.find(key)==data.end())cout<<clientId<<" element not exist"<<endl;
						else cout<<clientId<<" "<<data[key]<<endl;
					}
					else{
						if(mvcc_data.find(key)==mvcc_data.end())cout<<clientId<<" element not exist"<<endl;
						else cout<<clientId<<" "<<mvcc_data[key]<<endl;
					}
				}
				// this single command is a transaction
				else if(type=="DEL"){
					if(transactionReadonly){
						transactionReadonly=false;
//						lockMvcc.lock();
					}
					// 对mvcc副本操作
					unordered_map<string,string>::iterator it = mvcc_data.find(key);
					if(it==mvcc_data.end()){
						cout<<clientId<<" element not exist"<<endl;
						return 0;
					}
					string value=it->second;
					mvcc_data.erase(it);
					if(transactionBegin){
						// 记录undo log
						// undoLog.push("PUT "+key+" "+value);
					}else{
						// single command transaction 直接对原本进行原子操作 
						lockData.lock();
						data.erase(data.find(key));
						lockData.unlock();
						// lockMvcc.unlock();
						// 记录redo log
						writeRedoLog(cmd);
					} 
					cout<<clientId<<" DEL success"<<endl;
				}else if(type=="PUT"){
					string formular;
					string value;
					ss>>formular;
					value=calcFormularMvcc(formular);
					if(transactionReadonly){
						transactionReadonly=false;
//						lockMvcc.lock();
					}
					// 对mvcc副本操作
					unordered_map<string,string>::iterator it = mvcc_data.find(key);
					if(it!=mvcc_data.end()){
						// 覆盖原有数据 
						string oldValue=it->second;
						mvcc_data[key]=value;
						if(transactionBegin){
							// 记录undo log
							// undoLog.push("PUT "+key+" "+oldValue);
						}else{
							// single command transaction 直接对原本进行原子操作 
							lockData.lock();
							data[key]=value;
							lockData.unlock();
						}
					}else{
						// 直接添加数据
						mvcc_data[key]=value;
						if(transactionBegin){
							// 记录undo log
							// undoLog.push("DEL "+key);
						}else{
							// single command transaction 直接对原本进行原子操作 
							lockData.lock();
							data[key]=value;
							lockData.unlock();
							// 记录redo log
							writeRedoLog(cmd);					
						}
					}
					cout<<clientId<<" PUT success"<<endl;
				}
			}
			cmds[clientId].pop();
		}	
		Sleep(10);
	}
	return 0;
}
void executeRedo(){
	ifstream ifs("redo.log");
	string cmd;
	string clientId,type,key,value;
	stringstream ss;
	while(getline(ifs,cmd)){
		ss.clear();
		ss<<cmd;
		ss>>clientId>>type;
		if(type=="DEL"){
			ss>>key;
			data.erase(data.find(key));
		}else if(type=="PUT"){
			string formular;
			ss>>key>>formular;
			string value;
			value=calcFormularData(formular);
			data[key]=value;
		}
	}
	ifs.close();
}
void init(){
	// 读取文件
	data.clear();
	mvcc_data.clear();
	ifstream ifs("data.kvdb");
	string key,value;
	while(ifs>>key>>value)data[key]=value;
	ifs.close();
	// 检查redo log有没有东西
	executeRedo();
	mvcc_data=data;
	// 开启redo log
	ofsRedo.open("redo.log",ios::app);
}
int main(){
	init();
	string cmd;
	string clientId;
	stringstream ss;
	pthread_t t_toDisk;
	pthread_create(&t_toDisk, NULL, toDisk, NULL);
	while(getline(cin,cmd)){
		ss.clear();
		ss<<cmd;
		ss>>clientId;
		getline(ss,cmd);
		cmds[clientId].push(cmd);
		if(clientSet.find(clientId)==clientSet.end()){
			lockClientSet.lock();
			clientSet.insert(clientId);
			lockClientSet.unlock();
			pthread_t t;
		    int ret = pthread_create(&t, NULL, processThread, (void*)(clientId.data()));
		}
	}
	return 0;
}
