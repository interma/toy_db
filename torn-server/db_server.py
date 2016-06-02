# -*- coding: utf-8 -*-
import os,sys
file_path = os.path.split(os.path.realpath(__file__))[0]+"/"
sys.path.append(file_path+"../py-binding/")

import tornado.ioloop
import tornado.web
import tornado.httpserver
from bt_wrapper import BitcaskDB

db_path = file_path+"../test/server-bench/"
counter = 1 #per process var
db = BitcaskDB(db_path)
class DBHandler(tornado.web.RequestHandler):
    def get(self):
        global counter
        global db
        k = self.get_argument("k", None)
        if not k:
            self.write("i am running[%d]\n" % (counter))
            counter += 1
            return
        k = k.encode('utf8') # origin k is unicode type
        v = db.get(k)
        if v:
            self.write("get %s=>%s\n"%(k,v))
        else:
            self.write("get no key:%s\n"%(k))

    def post(self):
        k = self.get_body_argument("k", None)
        v = self.get_body_argument("v", None)
        if not k:
            self.write("key empty\n")
            return
        k = k.encode('utf8')
        if not v:
            db.dele(k)
            self.write("del %s\n"%(k))
        else:
            v = v.encode('utf8')
            db.set(k,v)
            self.write("set %s=>%s\n"%(k,v))

def make_app():
    return tornado.web.Application([
        (r"^/db/.*", DBHandler),
    ])

if __name__ == "__main__":
    db.open(False)
    app = make_app()
    server = tornado.httpserver.HTTPServer(app)
    server.bind(8888)
    # forks one process per cpu
    # but my db not support mulit process
    #server.start(0)  
    server.start(1)  
    tornado.ioloop.IOLoop.current().start()

