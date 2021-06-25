import requests
import uuid

'''
指令序列（循环10次）："PUT E 1","PUT F 1","PUT E (E+1)","PUT F (F+1)","GET E","DEL E","GET F","DEL F"
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
cmds = ["PUT E 1","PUT F 1","PUT E (E+1)","PUT F (F+1)","GET E","DEL E","GET F","DEL F"]

client_id = uuid.uuid4()
url = 'http://127.0.0.1:5000'
for i in range(10):
    for cmd in cmds:
        data = {'cmd':cmd, 'client_id':client_id}
        result = requests.post(url, data)
        print(result.text)


