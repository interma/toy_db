#BitcaskDB python ctypes wrapper

from ctypes import cdll
lib = cdll.LoadLibrary('../output/libtoydb.so')

class BitcaskDB(object):
	def __init__(self, db_path):
		self.obj = lib.BitcaskDB_new(db_path)
	def open(self, trunc=False):
		return lib.BitcaskDB_open(self.obj,trunc)
	def print_db(self):
		return lib.BitcaskDB_print_db(self.obj)
	def set(self, key, val):
		klen = len(key)
		vlen = len(val) # len-based string, not 0 ending
		return lib.BitcaskDB_set(self.obj,key,klen,val,vlen)

#test wrapper	
if __name__ == "__main__":
	db = BitcaskDB('../test/data/')
	db.open(False)
	ret = db.set("keypy", "xxx")
	img_data = open("../test/img.png").read()
	ret = db.set("imgpy", img_data)
	db.print_db()

# vim:noet:ts=4:sw=4:
