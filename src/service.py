from flask import Flask
from flask import request
from subprocess import Popen
from subprocess import PIPE
import subprocess
import threading
import time
import queue

class MessageReader(threading.Thread):
    stdout = None
    response_dict = None
    def __init__(self, stdout, response_dict):
        self.stdout = stdout
        self.response_dict = response_dict
        threading.Thread.__init__(self)
    def run(self) -> None:
        while not self.stdout.closed:
            try:
                res = self.stdout.readline()
                res = res.decode()
                res_list = res.split()
                if(len(res_list)<2):
                    continue
                key = res_list[0]
                if key not in self.response_dict:
                    self.response_dict[key] = queue.Queue()
                value = ' '.join(res_list[1:len(res_list)])
                self.response_dict[key].put(value)
            except ValueError:
                if self.stdout.closed:
                    return

path = 'KVDB.exe'
proc = Popen(path, stderr=subprocess.DEVNULL, shell=True, stdin=PIPE, stdout=PIPE)

response_dict = {}
reader = MessageReader(proc.stdout, response_dict)

reader.start()
# reader.join()

app = Flask(__name__)

def wait_for_response(client_id, timeout=60):
    key = client_id
    time_step = 0.05
    while timeout > 0:
        if key in response_dict:
            return response_dict[key].get(timeout=timeout)
        else:
            time.sleep(time_step)
            timeout -= time_step

@app.route('/',methods=['POST'])
def get_cmds():
    cmd = request.form['cmd']
    client_id = request.form['client_id']
    # print(cmd)
    # print(client_id)
    proc.stdin.write((client_id+' '+cmd+'\n').encode())
    proc.stdin.flush()
    res = wait_for_response(client_id)
    # res = response_dict[client_id].get(timeout=5)
    return res

app.debug = True  # 设置调试模式，生产模式的时候要关掉debug
app.run()
