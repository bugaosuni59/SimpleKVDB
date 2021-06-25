import requests
import uuid

'''
同时执行3_1 3_2 3_3
指令序列（循环10次）："PUT A 1","PUT B 1","PUT A (A+1)","PUT B (B+1)","GET A","DEL A","GET B","DEL B"
PUT success
PUT success
PUT success
PUT success
2
DEL success
2
DEL success
PUT success
PUT success
PUT success
PUT success
2
DEL success
2
DEL success
PUT success
PUT success
PUT success
PUT success
2
DEL success
2
DEL success
PUT success
PUT success
PUT success
PUT success
2
DEL success
2
DEL success
PUT success
PUT success
PUT success
PUT success
2
DEL success
2
DEL success
PUT success
PUT success
PUT success
PUT success
2
DEL success
2
DEL success
PUT success
PUT success
PUT success
PUT success
2
DEL success
2
DEL success
PUT success
PUT success
PUT success
PUT success
2
DEL success
2
DEL success
PUT success
PUT success
PUT success
PUT success
2
DEL success
2
DEL success
PUT success
PUT success
PUT success
PUT success
2
DEL success
2
DEL success
'''
cmds = ["PUT A 1","PUT B 1","PUT A (A+1)","PUT B (B+1)","GET A","DEL A","GET B","DEL B"]

client_id = uuid.uuid4()
url = 'http://127.0.0.1:5000'
for i in range(10):
    for cmd in cmds:
        data = {'cmd':cmd, 'client_id':client_id}
        result = requests.post(url, data)
        print(result.text)


