import tornado.ioloop
import tornado.web
import tornado.httpserver

class DBHandler(tornado.web.RequestHandler):
    def get(self):
        k = self.get_argument("k", None)
        if not k:
            self.write("i am running\n")
            return
        self.write("get %s\n"%(k))

    def post(self):
        k = self.get_body_argument("k", None)
        v = self.get_body_argument("v", None)
        if not k:
            self.write("key empty\n")
            return
        if not v:
            self.write("del %s\n"%(k))
        else:
            self.write("set %s=>%s\n"%(k,v))

def make_app():
    return tornado.web.Application([
        (r"^/db/.*", DBHandler),
    ])

if __name__ == "__main__":
    app = make_app()
    server = tornado.httpserver.HTTPServer(app)
    server.bind(8888)
    server.start(0)  # forks one process per cpu
    tornado.ioloop.IOLoop.current().start()

