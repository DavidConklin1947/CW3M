import pytest

def test_do_idu():
    with pytest.raises(SystemExit):
        do_idu("")
    assert(do_idu("C:\CW3M.git\trunk\DataCW3M\PEST_BLU9\IDU_BLU9")  == "Success.")

def test_do_lulc()):
    assert(test_do_lulc("") == "Not implemented yet.")


def test_do_wr():
    assert(test_do_lulc("") == "Not implemented yet.")
