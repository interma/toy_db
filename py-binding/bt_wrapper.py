from ctypes import cdll
lib = cdll.LoadLibrary('../output/libtoydb.so')

class BitcaskDB(object):
	def __init__(self, db_path):
		self.obj = lib.BitcaskDB_new(db_path)
	def open(self, trunc=False):
		return lib.BitcaskDB_open(self.obj,trunc)
	def print_db(self):
		return lib.BitcaskDB_print_db(self.obj)

	
if __name__ == "__main__":
	db = BitcaskDB('../test/data/')
	db.open(False)
	db.print_db()

