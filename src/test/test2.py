import requests
import uuid

'''
指令序列："GET A","GET B"
输出结果：
5
5
'''

cmds = ["GET A","GET B"]

client_id = uuid.uuid4()
url = 'http://127.0.0.1:5000'
for cmd in cmds:
    data = {'cmd':cmd, 'client_id':client_id}
    result = requests.post(url, data)
    print(result.text)


