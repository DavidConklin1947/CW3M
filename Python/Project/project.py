import shapefile
import os.path
import argparse
import csv
import sys

FIRST_NEW_IDU_ID = 164892
HBVCALIB_BLU = 9

def main():
    parser = argparse.ArgumentParser(prog = 'project', description="CW3M Utility")
    parser.add_argument('-b', "--basin", help='HBVCALIB value of watershed of interest', type=int, default=HBVCALIB_BLU)
    parser.add_argument('-d', "--do", help='do idu_id | lulc | wr', type=str, default="")
    parser.add_argument('-i', "--idu", help="base name of the IDU shapefile", type=str, default="")
    parser.add_argument('-l', "--lulc", help="name of the LULC XML file", type=str, default="")
    parser.add_argument('-w', "--wr", help="name of the points-of-diversion CSV file", type=str, default="")
    parser.add_argument('-n', "--next_id", help="the next unused idu_id value", type=int, default=FIRST_NEW_IDU_ID)
    args = parser.parse_args()
    args_dict = vars(args)
   
    what_to_do = ((args_dict["do"]).lower())
    print("what_to_do =", what_to_do)
    if len(what_to_do) <= 0:
        result = do_help()
    else:
        match what_to_do[0]:
            case 'b': # 'basin'
                base_name = args_dict["idu"]
                hbvcalib = args_dict["basin"]
                result = do_extract_basin(base_name, hbvcalib)
            case 'i': # 'idu'
                base_name = args_dict["idu"]
                first_new_idu_id = args_dict["next_id"]
                result = do_idu(base_name, first_new_idu_id)
            case 'l': # 'lulc'
                base_name = args.i
                lulc_file_name = args.l
                result = do_lulc(base_name, lulc_file_name)
            case 'w': # 'wr'
                pods_file_name = args.w
                result = do_wr(pods_file_name)
            case default:
                result = do_help()

    sys.exit(result)


def do_help():
    print("\nCW3M Utility")
    print("command line arguments are:")
    print("--basin hbvcalib")
    print("--do basin | idu_id | lulc | wr")
    print("--idu IDU_layer")
    print("--lulc LULC_spec")
    print("--next_id next_idu_id_value")
    print("--wr POD_file")
    return ""


def do_idu(base_name, first_new_idu_id):
    IDU_layer = shapefile.Reader(base_name)
    num_idus = len(IDU_layer)
    if num_idus <= 0:
        return "There are no IDUs."
    print("num_idus is", num_idus, "in", base_name)

    new_shapefile_loc = "NewFiles/" + base_name
    print(f"new shapefile is {new_shapefile_loc}")

    # Count up the degenerate IDUs with zero area.
    # Determine the highest-numbered IDU_ID.
    num_zero_area_idus = 0
    max_idu_id = -1
    ids_of_degenerate_idus = []
    for i in range(num_idus):
        idu = IDU_layer.record(i)
        if idu['AREA'] <= 0:
            idu_area = idu['AREA']
            idu_id = idu['IDU_ID']
            print(f"for i = {i} idu_id {idu_id} has area {idu_area}")
            num_zero_area_idus += 1
            ids_of_degenerate_idus.append(idu_id)
    print(f"num_zero_area_idus = {num_zero_area_idus}")
    print("max_idu_id is", max_idu_id, "in", base_name)
    if FIRST_NEW_IDU_ID <= max_idu_id:
        raise("max_idu_id >= FIRST_NEW_IDU_ID")

    # If there is a FID_Wetlan attribute, determine the number of IDUs
    # with non-negative fid_wetland values.
    num_non_neg_fid_wetlan = 0
    for i in range(num_idus):
        idu = IDU_layer.record(i)
        if int(idu['IDU_ID']) > max_idu_id:
            max_idu_id = int(idu['IDU_ID'])
        try:
            if idu['FID_Wetlan'] >= 0:
                num_non_neg_fid_wetlan += 1
        except IndexError:
            break # This shapefile doesn't have a "FID_Wetlan" attribute, so num_non_neg_fid_wetlan stays at 0
    print(f"num_non_neg_fid_wetlan = {num_non_neg_fid_wetlan}")

    print("\nLooking for duplicate IDU_IDs now. This can take a while...")
    num_duplicates = 0
    num_duplicates_in_BLU = 0
    next_new_idu_id = first_new_idu_id
    for i in range(num_idus - 1):
        idu_i = IDU_layer.record(i)
        try:
            fid_wetlan = idu_i['FID_Wetlan']
            if fid_wetlan == -1:
                continue
        except IndexError:
            pass
        idu_id_i = idu_i.IDU_ID
        hbvcalib_i = idu_i.HBVCALIB
        dup_found = False
        j = i + 1
        while j < num_idus:
            idu_j = IDU_layer.record(j)
            idu_id_j = idu_j.IDU_ID
            if idu_id_i != idu_id_j:
                if not dup_found and i % 2000 == 0:
                    print(i)
                break
            num_duplicates += 1
            if j == i + 1:
                idu_i.IDU_ID = next_new_idu_id
                print(i, "IDU_ID has been changed from", idu_id_i, " to", idu_i.IDU_ID)
                next_new_idu_id += 1
            idu_j.IDU_ID = next_new_idu_id
            print(j, "IDU_ID has been changed from", idu_id_j, " to", idu_j.IDU_ID)
            next_new_idu_id += 1
            dup_found = True
            j += 1

    print("first_new_idu_id is", first_new_idu_id)
    print('')
    print("num_idus is", num_idus, "in", base_name)
    print("num_non_neg_fid_wetlan is", num_non_neg_fid_wetlan)
    print("num_duplicates =", num_duplicates)

    if num_duplicates <= 0:
        print("No duplicates, so no new file will be created.")
    else:
        print(f"beginning to create new shapefile: {new_shapefile_loc}")
        writer = shapefile.Writer(new_shapefile_loc)
        writer.fields = IDU_layer.fields[1:]
        i = 0
        for shaperec in IDU_layer.iterShapeRecords():
            if shaperec.record['AREA'] > 0:
                writer.record(*shaperec.record)
                writer.shape(shaperec.shape)
            if i % 2000 == 0:
                print(f"working on record {i} now")
            i += 1
        print(f"{i} records processed. Closing now.")
        writer.close()

    return "Successful."


def do_extract_basin(base_name, hbvcalib):
    IDU_layer = shapefile.Reader(base_name)
    num_idus = len(IDU_layer)
    if num_idus <= 0:
        return "There are no IDUs."
    print("num_idus is", num_idus, "in", base_name)

    new_shapefile_loc = "NewFiles/" + "extract_from_" + base_name
    print(f"new shapefile is {new_shapefile_loc}")

    print(f"\nExtracting the basin of interest now. HBVCALIB = {hbvcalib}. This can take a while...")
    num_idus_in_basin = 0
    writer = shapefile.Writer(new_shapefile_loc)
    writer.fields = IDU_layer.fields[1:]
    i = 0
    for shaperec in IDU_layer.iterShapeRecords():
        if shaperec.record['AREA'] > 0 and shaperec.record['HBVCALIB'] == hbvcalib:
            writer.record(*shaperec.record)
            writer.shape(shaperec.shape)
            num_idus_in_basin += 1
        if i % 2000 == 0:
            print(f"working on record {i} now")
        i += 1
    print(f"{i} records processed. New file has {num_idus_in_basin}. Closing now.")
    writer.close()

    return "Successful."


def do_lulc():
    return "Not implemented yet."


def do_wr():
    return "Not implemented yet."


if __name__ == "__main__":
    main()