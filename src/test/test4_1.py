import requests
import uuid
import time
'''
开3个这个程序，循环运行1分钟后关闭，运行4_2
指令序列："BEGIN","PUT A (A+1)","PUT B (B+1)","PUT A (A+1)","PUT B (B+1)","PUT A (A+1)","PUT A (A+1)","PUT B (B+1)","PUT B (B+1)","PUT A (A+1)","PUT B (B+1)","COMMIT","BEGIN","PUT A (A+1)","PUT B (B+1)","PUT A (A+1)","PUT B (B+1)","PUT A (A+1)","PUT A (A+1)","PUT B (B+1)","PUT B (B+1)","PUT A (A+1)","PUT B (B+1)","COMMIT","BEGIN","PUT A (A-1)","PUT B (B-1)","PUT A (A-1)","PUT B (B-1)","PUT A (A-1)","PUT A (A-1)","PUT B (B-1)","PUT B (B-1)","PUT A (A-1)","PUT B (B-1)","ABORT"
'''
cmds = ["BEGIN","PUT A (A+1)","PUT B (B+1)","PUT A (A+1)","PUT B (B+1)","PUT A (A+1)","PUT A (A+1)","PUT B (B+1)","PUT B (B+1)","PUT A (A+1)","PUT B (B+1)","COMMIT","BEGIN","PUT A (A+1)","PUT B (B+1)","PUT A (A+1)","PUT B (B+1)","PUT A (A+1)","PUT A (A+1)","PUT B (B+1)","PUT B (B+1)","PUT A (A+1)","PUT B (B+1)","COMMIT","BEGIN","PUT A (A-1)","PUT B (B-1)","PUT A (A-1)","PUT B (B-1)","PUT A (A-1)","PUT A (A-1)","PUT B (B-1)","PUT B (B-1)","PUT A (A-1)","PUT B (B-1)","ABORT"]

client_id = uuid.uuid4()
url = 'http://127.0.0.1:5000'
time_start = time.time()
while True:
    for cmd in cmds:
        data = {'cmd':cmd, 'client_id':client_id}
        result = requests.post(url, data)
        print(result.text)
    time_end = time.time()
    if(time_end-time_start>=60):
        break


