import requests
import uuid
'''
指令序列："PUT A 3","PUT B 4","GET A","GET B","DEL A","DEL B","PUT A 5","GET A","GET B","PUT B 5","GET B"
输出结果：
    PUT success
    PUT success
    3
    4
    DEL success
    DEL success
    PUT success
    5
    element not exist
    PUT success
    5
    
    Process finished with exit code 0
'''
cmds = ["PUT A 3","PUT B 4","GET A","GET B","DEL A","DEL B","PUT A 5","GET A","GET B","PUT B 5","GET B"]
client_id = uuid.uuid4()
url = 'http://127.0.0.1:5000'
for cmd in cmds:
    data = {'cmd':cmd, 'client_id':client_id}
    result = requests.post(url, data)
    print(result.text)



# from flask import Flask
# from flask import request
# from subprocess import Popen
# from subprocess import PIPE
# import subprocess
# import threading
# import time
# import queue
#
# class MessageReader(threading.Thread):
#     stdout = None
#     response_dict = None
#     def __init__(self, stdout, response_dict):
#         self.stdout = stdout
#         self.response_dict = response_dict
#         threading.Thread.__init__(self)
#     def run(self) -> None:
#         while not self.stdout.closed:
#             try:
#                 res = self.stdout.readline()
#                 res = res.decode()
#                 res_list = res.split()
#                 if(len(res_list)<2):
#                     continue
#                 key = res_list[0]
#                 if key not in self.response_dict:
#                     self.response_dict[key] = queue.Queue()
#                 value = ' '.join(res_list[1:len(res_list)])
#                 self.response_dict[key].put(value)
#             except ValueError:
#                 if self.stdout.closed:
#                     return
#
# path = 'E:\\recent\\2021\\dsj大数据分析与内存计算\\final\\KVDB.exe'
# # path = 'E:\\recent\\2021\\dsj大数据分析与内存计算\\final\\testpy.exe'
# # path = 'E:\\recent\\2021\\dsj大数据分析与内存计算\\final\\echo.exe'
# proc = Popen(path, stderr=subprocess.DEVNULL, shell=True, stdin=PIPE, stdout=PIPE)
#
# response_dict = {}
# reader = MessageReader(proc.stdout, response_dict)
#
# reader.start()
# # reader.join()
#
#
# def wait_for_response(client_id, timeout=5):
#     key = client_id
#     time_step = 0.05
#     while timeout > 0:
#         if key in response_dict:
#             return response_dict[key].get(timeout=timeout)
#         else:
#             time.sleep(time_step)
#             timeout -= time_step
#
# client_id="id1"
# cmds=["PUT A 1","GET A"]
# for cmd in cmds:
#     proc.stdin.write((client_id+' '+cmd+'\n').encode())
#     proc.stdin.flush()
#     res = wait_for_response(client_id)
#     print(res)
#
#
