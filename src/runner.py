import requests
import uuid
'''
data = {'cmd':'PUT A B','client_id':'uid2'}
# data = {'cmd':'PUT A B'}
r = requests.post(url,data)
print(r.text)
'''
client_id = uuid.uuid4()
url = 'http://127.0.0.1:5000'
while True:
    cmd = input()
    data = {'cmd':cmd, 'client_id':client_id}
    result = requests.post(url, data)
    print(result.text)
