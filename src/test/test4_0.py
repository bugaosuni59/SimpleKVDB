import requests
import uuid

'''
运行该程序后运行4_1
指令序列："PUT A 1","PUT B 1"
PUT success
PUT success
'''
cmds = ["PUT A 1","PUT B 1"]

client_id = uuid.uuid4()
url = 'http://127.0.0.1:5000'
for cmd in cmds:
    data = {'cmd':cmd, 'client_id':client_id}
    result = requests.post(url, data)
    print(result.text)


