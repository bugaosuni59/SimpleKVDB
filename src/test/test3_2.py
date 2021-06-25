import requests
import uuid
'''
同时执行3_1 3_2 3_3
指令序列（循环10次）："PUT C 1","PUT D 1","PUT C (C+1)","PUT D (D+1)","GET C","DEL C","GET D","DEL D"
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

Process finished with exit code 0

'''
cmds = ["PUT C 1","PUT D 1","PUT C (C+1)","PUT D (D+1)","GET C","DEL C","GET D","DEL D"]

client_id = uuid.uuid4()
url = 'http://127.0.0.1:5000'
for i in range(10):
    for cmd in cmds:
        data = {'cmd':cmd, 'client_id':client_id}
        result = requests.post(url, data)
        print(result.text)


