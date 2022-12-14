# WRcount - Tallies up kinds of water rights in the PODs file
# a water rights utility for CW3M

import argparse
import csv
import sys

# water right use codes
WRU_IRRIGATION=16
WRU_MUNICIPAL=1024
WRU_INSTREAM=2048

# water right permit codes
WRP_SURFACE=2
WRP_GROUNDWATER=4

FIRST_ARTIFICIAL_WATERRIGHTID=1000000
MAX_UGA_NDX=72


def main():
    parser = argparse.ArgumentParser(prog = 'WRcount', description="CW3M Utility")
    parser.add_argument('-d', "--diversions", help="name of the points-of-diversion CSV file", type=str)
    parser.add_argument('-a', "--artificial", help="lowest artificial water right ID number", type=int, default=FIRST_ARTIFICIAL_WATERRIGHTID)
    args = parser.parse_args()
    args_dict = vars(args)

    if len(sys.argv) < 2:
        local_help()
        sys.exit()

    pods_file_name = args_dict["diversions"]
    first_artificial_waterrightid = args_dict['artificial']
    result = wr_count(pods_file_name, first_artificial_waterrightid)

    sys.exit(result)


def local_help():
    print("\nWRcount, one of the CW3M Utilities")
    print("command line arguments are:")
    print('--diversions POD_file   for example "wr_pods.csv"')
    print('--artificial first_artificial_waterrightid      default="1000000"')
    print()
    return ""


def wr_count(file_name, first_artificial_waterrightid): 
    pod_file = open(file_name)
    pod_reader = csv.reader(pod_file)
    header_list = next(pod_reader)

    num_cols = len(header_list)
    num_records = 0
    num_artificial = 0
    num_artificial_muni = 0
    num_artificial_instream = 0
    num_real = 0
    num_muni = 0
    num_muni_ugb = 0
    num_muni_ugb_surface = 0
    num_muni_ugb_groundwater = 0
    num_irrigation = 0
    num_instream = 0
    num_non_int_pouids = 0
    num_non_int_ugbs = 0

    waterrightid_ndx = header_list.index("WATERRIGHTID")
    usecode_ndx = header_list.index("USECODE")
    permitcode_ndx = header_list.index("PERMITCODE")
    pouid_ndx = header_list.index("POUID")
    ugb_ndx = -1
    try:
        ugb_ndx = header_list.index("UGB")
    except ValueError:
        pass

    while row := next(pod_reader, False):
        num_records += 1
        waterrightid = int(row[waterrightid_ndx])
        usecode = int(row[usecode_ndx])
        permitcode = int(row[permitcode_ndx])

        if waterrightid >= first_artificial_waterrightid:
            num_artificial += 1
            if usecode == WRU_MUNICIPAL:
                num_artificial_muni += 1
            elif usecode == WRU_INSTREAM:
                num_artificial_instream += 1
        else:
            num_real += 1
            if usecode == WRU_MUNICIPAL:
                num_muni += 1
                ugb = -9999
                if ugb_ndx >= 0:
                    try:
                        ugb = int(row[ugb_ndx])
                    except ValueError:
                        num_non_int_ugbs += 1
                else:
                    try:
                        pouid = int(row[pouid_ndx])
                    except ValueError:
                        num_non_int_pouids += 1
                        pouid = -9999
                    if pouid < 0 and -pouid <= MAX_UGA_NDX:
                        ugb = -pouid
                if 0 < ugb <= MAX_UGA_NDX:
                    num_muni_ugb += 1
                    if permitcode == WRP_SURFACE:
                        num_muni_ugb_surface += 1
                    elif permitcode == WRP_GROUNDWATER:
                        num_muni_ugb_groundwater += 1
            elif usecode == WRU_IRRIGATION:
                num_irrigation += 1
            elif usecode == WRU_INSTREAM:
                num_instream += 1

    print(f"{file_name} has {num_cols} columns and {num_records} data records")
    print(f"    {num_artificial} are artificial (waterrightid >= {first_artificial_waterrightid}), of which")
    print(f"        {num_artificial_muni} are for municipal use, and")
    print(f"        {num_artificial_instream} are for instream water rights")
    print(f"    {num_real} are from OWRD, of which")
    print(f"        {num_irrigation} are for irrigation,")
    print(f"        {num_instream} are instream water rights, and")
    print(f"        {num_muni} are for municipal use. Of these muni right records,")
    if ugb_ndx >= 0:
        print(f"            {num_non_int_ugbs} have non-integer UGB values")
    else:
        print(f"            {num_non_int_pouids} have non-integer POUID values")
    print(f"            {num_muni_ugb} are for UGAs. Of these urban muni records,")
    print(f"                {num_muni_ugb_surface} are surface water rights, and")
    print(f"                {num_muni_ugb_groundwater} are groundwater rights")


if __name__ == "__main__":
    main()

