#BitcaskDB python ctypes wrapper

import os
from ctypes import *
file_path = os.path.split(os.path.realpath(__file__))[0]+"/"
lib = cdll.LoadLibrary(file_path+'../output/libtoydb.so')

class BitcaskDB(object):
	def __init__(self, db_path):
		self.obj = lib.BitcaskDB_new(db_path)
	def open(self, trunc=False):
		return lib.BitcaskDB_open(self.obj,trunc)
	def destory(self):
		return lib.BitcaskDB_destory(self.obj)
	def print_db(self):
		return lib.BitcaskDB_print_db(self.obj)
	def set(self, key, val):
		klen = len(key)
		vlen = len(val) # len-based string, not 0 ending
		return lib.BitcaskDB_set(self.obj,key,klen,val,vlen)
	def dele(self, key):
		klen = len(key)
		return lib.BitcaskDB_del(self.obj,key,klen)
	def get(self, key, vlen=1024):
		klen = len(key)
		buf = create_string_buffer(vlen)
		pot = addressof(buf)
		ret = lib.BitcaskDB_get(self.obj,key,klen,pot,vlen)
		if ret < 0:
			return None
		return string_at(pot,ret)	

#test wrapper	
if __name__ == "__main__":
	db = BitcaskDB(file_path+'../test/data/')
	db.open(False)
	ret = db.set("keypy", "xxx")
	img_data = open(file_path+"../test/img.png").read()
	ret = db.set("imgpy", img_data)
	key = "keypy"
	print "GET[%s]:%s" % (key,db.get(key))
	key = "imgpy"
	print "GET[%s]:%d" % ( key,len(db.get(key,1024*1024)) )
	ret = db.dele("keypy")
	db.print_db()
	db.destory()

# vim:noet:ts=4:sw=4:
