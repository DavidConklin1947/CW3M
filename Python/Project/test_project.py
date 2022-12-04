import pytest
from project import do_extract
from project import do_idu
from project import do_lulc
from project import do_wr


def test_do_extract():
    assert(do_extract("IDU_BLU", "VEGCLASS", 152) == 2)

def test_do_idu():
   assert(do_idu("IDU_BLU", 164892)  == 49)

def test_do_lulc():
    assert(do_lulc("IDU_BLU", "idu.xml") == 0)

def test_do_wr():
    assert(do_wr("wr_pods.csv") == 22057)

def main():
    print(do_extract("IDU_BLU", "VEGCLASS", 152))
    print(do_idu("IDU_BLU", 164892))
    print(do_lulc("IDU_BLU", "idu.xml"))
    print(do_wr("wr_pods.csv"))

if __name__ == "__main__":
    main()