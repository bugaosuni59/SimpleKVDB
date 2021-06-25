#include <bits/stdc++.h>
#include <pthread.h>
#include <Windows.h>
#include <mutex>
using namespace std;
/*
�ڴ����������汾��
	ԭ�����ݣ�ֻ����MVCC��д 
	MVCC��������д
�ؼ����ݣ�
	�����ļ���ÿһ��ʱ������һ�Σ���ԭʼ���ݿ�ʼִ�в����redo log
	undo log��transaction�����м�¼������������abort��ִ��һ�鷴�����������Ҫ���̣������ڴ��Ｔ��
		����ֱ����MVCC������ԭ������ʵ�֣�����Ҫ��¼undo log
	redo log�����̼��ڵ��κβ���
��������������������redo logʵ������һ����
�����ļ���������ʱ��ס 
redo log�����Ѳ�����¼���ļ�ʱ��ס 
	
֧�ֲ��롢ɾ��������
    ���ʽ������
֧��ACID
    Atomicityԭ����
        undo log:
            ����PUT��������¼��Ӧ��DEL����
            ����DEL��������¼��Ӧ��PUT����
        transaction��ʼ��ʱ��ʹ�һ��undo log
        �յ�abort��ʱ�����һ��undo log
    Consistencyһ����
        ���ݿ��״̬��ִ?��Ԥ�ڽ��?��
        consistent state before and after transactions
        �趨����ԭ��
            ʵ�������ԣ�keyΨһ
            �������ԣ�value size������65535(64KB)
    Isolation������
        ���ݲ�ͬ�ĸ��뼶��ʵ������֮��Ļ���?�ţ���Ҫ���������㷨
        ���������㷨MVCC:
            ÿ��transaction���ж���ֻ�����Ƕ�д��
            ����Ƕ�д���ȼ��copyȨ������û���Ļ�copyһ�����ݣ���copy����������������Ȼ����סԭ���ģ��滻��
            ���˵Ļ�����copy��������
            �����ֻ������ԭ�汾��������
    Durability�־û�
        redo log
            �����κβ�������¼��log��
            COMMITʱ�����ż�����
            ����һ�����ʱ�䣬����10�룬ÿ10����һ���̣����redo log
            ������ʱ����checkһ����û��û����redo log
*/
/*
֧�ֵ����
    PUT A 3
    GET A
    DEL A
    PUT C (C+1)
    BEGIN
    COMMIT
    BEGIN
    ABORT
Э�飺
	uid command
	uid DISCONNECT
*/
unordered_map<string,string> data;// ԭʼ���� 
unordered_map<string,string> mvcc_data;// MVCC���� 
unordered_map<string,queue<string>> cmds;// ÿ���ͻ��˵�����ָ�� 
set<string>clientSet;// �ͻ��˼��� 
int timeToDisk=10;// ���̼�� 
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
	// data���̣����redo log
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
	queue<string>redoLogTrans;// ��ǰtransaction��redo log
	bool transactionBegin=false;// �Ƿ�����һ��transactoin 
	bool transactionReadonly=true;// ��ǰTransaction�Ƿ�һֱ��read 
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
				// ��ʼ��¼undo log
				if(!transactionBegin){
					transactionBegin=true;
				}
				lockMvcc.lock();
				cout<<clientId<<" begin a transaction"<<endl;
			}else if(type=="COMMIT"){
				// ɾ��undo log������ԭ�ӵ�д��ԭ��
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
					// ��mvcc��������
					unordered_map<string,string>::iterator it = mvcc_data.find(key);
					if(it==mvcc_data.end()){
						cout<<clientId<<" element not exist"<<endl;
						return 0;
					}
					string value=it->second;
					mvcc_data.erase(it);
					if(transactionBegin){
						// ��¼undo log
						// undoLog.push("PUT "+key+" "+value);
					}else{
						// single command transaction ֱ�Ӷ�ԭ������ԭ�Ӳ��� 
						lockData.lock();
						data.erase(data.find(key));
						lockData.unlock();
						// lockMvcc.unlock();
						// ��¼redo log
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
					// ��mvcc��������
					unordered_map<string,string>::iterator it = mvcc_data.find(key);
					if(it!=mvcc_data.end()){
						// ����ԭ������ 
						string oldValue=it->second;
						mvcc_data[key]=value;
						if(transactionBegin){
							// ��¼undo log
							// undoLog.push("PUT "+key+" "+oldValue);
						}else{
							// single command transaction ֱ�Ӷ�ԭ������ԭ�Ӳ��� 
							lockData.lock();
							data[key]=value;
							lockData.unlock();
						}
					}else{
						// ֱ���������
						mvcc_data[key]=value;
						if(transactionBegin){
							// ��¼undo log
							// undoLog.push("DEL "+key);
						}else{
							// single command transaction ֱ�Ӷ�ԭ������ԭ�Ӳ��� 
							lockData.lock();
							data[key]=value;
							lockData.unlock();
							// ��¼redo log
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
	// ��ȡ�ļ�
	data.clear();
	mvcc_data.clear();
	ifstream ifs("data.kvdb");
	string key,value;
	while(ifs>>key>>value)data[key]=value;
	ifs.close();
	// ���redo log��û�ж���
	executeRedo();
	mvcc_data=data;
	// ����redo log
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
