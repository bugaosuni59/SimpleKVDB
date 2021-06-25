import requests
import uuid

cmds = ["GET A","GET B"]
'''
指令序列："GET A","GET B"
1171
1171
'''
client_id = uuid.uuid4()
url = 'http://127.0.0.1:5000'
for cmd in cmds:
    data = {'cmd':cmd, 'client_id':client_id}
    result = requests.post(url, data)
    print(result.text)


