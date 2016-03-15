import requests
import time
import datetime

while True:
    print datetime.datetime.utcnow()
    try:
        res = requests.post("http://192.168.1.19", data="6,1", timeout=10)
        print res.text
    except Exception as e:
        print e
    time.sleep(1)
    try:
        res = requests.post("http://192.168.1.19", data="6,0", timeout=10)
        print res.text
    except Exception as e:
        print e
    time.sleep(1)
