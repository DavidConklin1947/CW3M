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


def do_wr(new_file_name): # Return the number of data records in the merged POD file.
    # "old file" = the original POD file, named "wr_pods.csv", located in the current folder
    # "new file" = the new POD file before processing for compatibility with CW3M, located in the current folder
    # "merged file" = the POD file with data from both old and new files, processed for compatibility with CW3M
    # The merged file is located in ./NewFiles and has the name wr_pods.csv.
    # Start a merged POD file in the NewFiles folder.
    # Copy the column headers from the old file to the merged file.
    # Add UGB and CW3M_YEAR column headers.

    # water right use codes
    WRU_IRRIGATION=16
    WRU_MUNICIPAL=1024
    WRU_INSTREAM=2048

    # water right permit codes
    WRP_SURFACE=2
    WRP_GROUNDWATER=4

    FIRST_ARTIFICIAL_WATERRIGHTID=1000000

    old_file = open("wr_pods.csv")
    old_reader = csv.reader(old_file) 
    old_header_list = next(old_reader) 
    print("old_header_list:", old_header_list)

    new_file = open(new_file_name)
    new_reader = csv.reader(new_file)
    new_header_list = next(new_reader) # This call initializes new_reader.fieldnames
    print("new_header_list:", new_header_list)

    if old_header_list != new_header_list:
        print(f"old_header_list = {old_header_list}")
        print("new_header_list =", new_header_list)

    header_list = old_header_list
    if not "UGB" in header_list:
        header_list.append("UGB")
    if not "CW3M_YEAR" in header_list:
        header_list.append("CW3M_YEAR")
    print("header_list for merged_file:", header_list)

    merged_file = open("NewFiles/merged_file.csv", 'w', newline='')
    print("just before csv.writer() call:", header_list)
    merged_writer = csv.writer(merged_file)
    merged_writer.writerow(header_list)

    new_count = 0
    merged_muni = {} # Create an empty dictionary of municipal-use water rights, keyed by their water right IDs.
    # Note that these dictionary entries will be lists, because some water right IDs have more than
    # one point-of-diversion.
    year_ndx = header_list.index("YEAR")
    usecode_ndx = header_list.index("USECODE")
    waterrightid_ndx = header_list.index("WATERRIGHTID")
    while merged_row_list:= next(new_reader, False):
        merged_row_list.append("0") # UGB
        merged_row_list.append(merged_row_list[year_ndx]) # CW3M_YEAR
        # if this is not a municipal use water right, just copy it into the merged file.
        if int(merged_row_list[usecode_ndx]) != WRU_MUNICIPAL:
            merged_writer.writerow(merged_row_list)
        else: # If it is a muni wr, then
            waterrightid_str = merged_row_list[waterrightid_ndx]
            if waterrightid_str not in merged_muni:
                merged_muni[waterrightid_str] = [merged_row_list] 
            else:
                (merged_muni[waterrightid_str]).append(merged_row_list)
        new_count += 1
   
    # Now process the muni water rights.
    # If there are corresponding water right IDs in the old file, copy their UGB and 
    # priority year data into the new UGB and CW3M_YEAR fields.
    # Copy all the old artificial muni water right records to the new muni dictionary.
    print("Beginning to merge old and new water rights now...")
    pouid_ndx = header_list.index("POUID")
    ugb_ndx = header_list.index("UGB")
    cw3m_year_ndx = header_list.index("CW3M_YEAR")
    for row in old_reader:
        old_waterrightid_str = row[waterrightid_ndx]
        old_waterrightid_int = int(old_waterrightid_str)
        if old_waterrightid_str in merged_muni:
            if not (int(row[usecode_ndx]) == WRU_MUNICIPAL):
                print("old_waterrightid_str =", old_waterrightid_str, " row[usecode_ndx] =", row[usecode_ndx])
            pouid = int(row[pouid_ndx])
            if (pouid < 0):
                ugb = -pouid
                pod_list = merged_muni[old_waterrightid_str]
                merged_pod_list = []
                for pod_record in pod_list:
                    merged_pod_record = pod_record
                    merged_pod_record[ugb_ndx] = ugb
                    cw3m_year = row[year_ndx]
                    merged_pod_record[cw3m_year_ndx] = cw3m_year
                    merged_pod_list.append(merged_pod_record)
                merged_muni[old_waterrightid_str] = merged_pod_list
        elif old_waterrightid_int >= FIRST_ARTIFICIAL_WATERRIGHTID:
            # Copy the artificial water rights to the merged list and fix the UGB and CW3M_YEAR columns.
            # ??? What about the artificial water rights that are not for municipal use? Do they have associated POU records?
            merged_pod_record = row
            pouid = int(merged_pod_record[pouid_ndx])
            if (pouid < 0):
                ugb = -pouid
            else:
                ugb = 0
            merged_pod_record.append(ugb) 
            merged_pod_record.append(merged_pod_record[year_ndx])
            merged_writer.writerow(merged_pod_record) 

    # Now add the merged muni water rights back into the merged_file.
    print("merged_muni:")
    merged_muni_keys = merged_muni.keys()
    for key in merged_muni_keys:
        wr_id_list = merged_muni[key]
        print("wr_id:", key)
        for row in wr_id_list:
            print("  ", row)
            merged_writer.writerow(row)
 
    print(f"There are {new_count} data rows in {new_file_name}")
    return new_count


if __name__ == "__main__":
    main()

