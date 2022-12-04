import shapefile
import os.path
import argparse
import csv
import sys

FIRST_NEW_IDU_ID = 164892
SHRUBLAND = 152

def main():
    parser = argparse.ArgumentParser(prog = 'project', description="CW3M Utility")
    parser.add_argument('-a', "--attribute", help='name of attribute of interest, defaults to UGB', type=str, default="VEGCLASS")
    parser.add_argument('-d', "--do", help='do idu_id | lulc | wr', type=str, default="")
    parser.add_argument('-i', "--idu", help="base name of the IDU shapefile", type=str, default="")
    parser.add_argument('-l', "--lulc", help="name of the LULC XML file", type=str, default="")
    parser.add_argument('-v', "--value_of_attribute", help='value of attribute of interest, defaults to the value for Eugene-Springfield', type=int, default=SHRUBLAND)
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
            case 'e': # 'extract'
                base_name = args_dict["idu"]
                attribute_name = args_dict["attribute"]
                attribute_value = args_dict["value_of_attribute"]
                result = do_extract_basin(base_name, attribute_name, attribute_value)
            case 'i': # 'idu'
                base_name = args_dict["idu"]
                first_new_idu_id = args_dict["next_id"]
                result = do_idu(base_name, first_new_idu_id)
            case 'l': # 'lulc'
                base_name = args.i
                lulc_file_name = args.l
                result = do_lulc(base_name, lulc_file_name)
            case 'w': # 'wr'
                pods_file_name = args_dict["wr"]
                result = do_wr(pods_file_name)
            case default:
                result = do_help()

    sys.exit(result)


def do_help():
    print("\nCW3M Utility")
    print("command line arguments are:")
    print("--attribute name_of_attribute")
    print("--do extract | idu_id | lulc | wr")
    print("--idu IDU_layer")
    print("--lulc LULC_spec")
    print("--next_id next_idu_id_value")
    print("--value_of_attribute value")
    print("--wr POD_file")
    return ""


def do_idu(base_name, first_new_idu_id): # Return the number of duplicate IDUs.
    IDU_layer = shapefile.Reader(base_name)
    num_idus = len(IDU_layer)
    if num_idus <= 0:
        return "There are no IDUs."
    print("num_idus is", num_idus, "in", base_name)

    new_shapefile_loc = "NewFiles/" + base_name
    print(f"new shapefile is {new_shapefile_loc}")
    writer = shapefile.Writer(new_shapefile_loc)
    writer.fields = IDU_layer.fields[1:]

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
        except AttributeError | IndexError:
            break # This shapefile doesn't have a "FID_Wetlan" attribute, so num_non_neg_fid_wetlan stays at 0
    print(f"num_non_neg_fid_wetlan = {num_non_neg_fid_wetlan}")

    print("\nLooking for duplicate IDU_IDs now. This can take a while...")
    num_duplicates = 0
    num_duplicated = 0
    next_new_idu_id = first_new_idu_id
    i = 0
    while i < (num_idus - 1):
        idu_i = IDU_layer.record(i)
        idu_id_i = idu_i.IDU_ID
        dup_found = False
        j = i + 1
        while j < num_idus:
            idu_j = IDU_layer.record(j)
            idu_id_j = idu_j.IDU_ID
            if idu_id_i != idu_id_j:
                if j == i + 1:
                    writer.record(*idu_i)
                    writer.shape(IDU_layer.shape(i))
                if not dup_found and i % 2000 == 0:
                    print(i)

                break
            num_duplicates += 1
            if j == i + 1:
                idu_i.IDU_ID = next_new_idu_id
                num_duplicated += 1
                print(i, "IDU_ID has been changed from", idu_id_i, " to", idu_i.IDU_ID)
                next_new_idu_id += 1
                writer.record(*idu_i)
                writer.shape(IDU_layer.shape(i))
            idu_j.IDU_ID = next_new_idu_id
            print(j, "IDU_ID has been changed from", idu_id_j, " to", idu_j.IDU_ID)
            next_new_idu_id += 1
            dup_found = True
            writer.record(*idu_j)
            writer.shape(IDU_layer.shape(j))
            j += 1
        i = j
    print(f"{num_idus} records processed. Closing now.")
    writer.close()
    print("first_new_idu_id is", first_new_idu_id)
    print('')
    print("num_idus is", num_idus, "in", base_name)
    print("num_non_neg_fid_wetlan is", num_non_neg_fid_wetlan)
    print("num_duplicates =", num_duplicates)
    print("num_duplicated =", num_duplicated)

    return num_duplicates


def do_extract(base_name, attribute_name, attribute_value): # Return the number IDUs in the extract.
    IDU_layer = shapefile.Reader(base_name)
    num_idus = len(IDU_layer)
    if num_idus <= 0:
        return "There are no IDUs."
    print("num_idus is", num_idus, "in", base_name)

    new_shapefile_loc = "NewFiles/" + "extract_from_" + base_name
    print(f"new shapefile is {new_shapefile_loc} using attribute {attribute_name} = {attribute_value}")

    print(f"\nExtracting now. This can take a while...")
    num_idus_in_extract = 0
    writer = shapefile.Writer(new_shapefile_loc)
    writer.fields = IDU_layer.fields[1:]
    i = 0
    for shaperec in IDU_layer.iterShapeRecords():
        if shaperec.record['AREA'] > 0 and shaperec.record[attribute_name] == attribute_value:
            writer.record(*shaperec.record)
            writer.shape(shaperec.shape)
            num_idus_in_extract += 1
        if i % 2000 == 0:
            print(f"working on record {i} now")
        i += 1
    print(f"{i} records processed. New file has {num_idus_in_extract} IDUs. Closing now.")
    writer.close()

    return num_idus_in_extract


def do_lulc(base_name, lulc_file_name): # Return the number of IDUs for which LULC_A/LULC_B/VEGCLASS values were changed.
    return 0


def do_wr(pods_file_name): # Return the number of data records in the new POD file.
    file = open(pods_file_name)
    reader = csv.reader(file)
    count = 0
    for _ in reader:
        count += 1

    count -= 1 # Don't count the header record.
    print(f"There are {count} data rows in {pods_file_name}")
    return count


if __name__ == "__main__":
    main()