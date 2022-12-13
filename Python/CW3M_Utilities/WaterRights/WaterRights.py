# water_rights utility for CW3M

# import os.path
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


def main():
    parser = argparse.ArgumentParser(prog = 'water_rights', description="CW3M Utility")
    parser.add_argument('-m', "--merged", help="name of the merged points-of-diversion CSV file", type=str, default="NewFiles/merged_wr_pods.csv")
    parser.add_argument('-n', "--new", help="name of the new points-of-diversion CSV file", type=str, default="")
    parser.add_argument('-o', "--old", help="name of the old points-of-diversion CSV file", type=str, default="./wr_pods.csv")
    args = parser.parse_args()
    args_dict = vars(args)

    
    merged_pods_file_name = args_dict["merged"]
    new_pods_file_name = args_dict["new"]
    old_pods_file_name = args_dict["old"]
    if len(new_pods_file_name) <= 0:
        result = do_help()
        sys.exit()
    result = do_wr(old_pods_file_name, new_pods_file_name, merged_pods_file_name)

    sys.exit(result)


def do_help():
    print("\nWaterRights, one of the CW3M Utilities")
    print("command line arguments are:")
    print('--merged merged_POD_file   default="NewFiles/merged_wr_pods.csv"')
    print('--new new_POD_file   required, no default')
    print('--old old_POD_file   default="./wr_pods.csv"')
    print()
    return ""


def do_wr(old_pods_file_name, new_pods_file_name, merged_pods_file_name): # Return the number of data records in the merged POD file.
    # "old file" = the original POD file
    # "new file" = the new POD file before processing for compatibility with CW3M
    # "merged file" = the POD file with data from both old and new files, processed for compatibility with CW3M
    # Start a merged POD file.
    # Copy the column headers from the old file to the merged file.
    # Add UGB and CW3M_YEAR column headers.
    # From the new pods file, create a dictionary of municipal-use water rights, keyed by their water right IDs.
    # Note that the dictionary entries will be lists, because some water right IDs have more than one point-of-diversion.
    # From the old pods file, for any muni water right whose WATERRIGHTID value is also in the new file,
    # copy the UGB and CW3M_YEAR values from the old file to the dictionary entries.
    # Copy the artificial water rights from the old file to the dictionary or directly
    # to the merged file.
    # Add the merged muni rights from the dictionary to the merged file.

    old_file = open(old_pods_file_name)
    old_reader = csv.reader(old_file) 
    old_header_list = next(old_reader) 
#    print("old_header_list:", old_header_list)

    new_file = open(new_pods_file_name)
    new_reader = csv.reader(new_file)
    new_header_list = next(new_reader) 
#    print("new_header_list:", new_header_list)

    if old_header_list != new_header_list:
        print(f"old_header_list = {old_header_list}")
        print("new_header_list =", new_header_list)

    header_list = old_header_list
    if not "UGB" in header_list:
        header_list.append("UGB")
    if not "CW3M_YEAR" in header_list:
        header_list.append("CW3M_YEAR")
    print("header_list for merged_file:", header_list)

    merged_file = open(merged_pods_file_name, 'w', newline='')
    print("just before csv.writer() call:", header_list)
    merged_writer = csv.writer(merged_file)
    merged_writer.writerow(header_list)

    new_count = 0
    new_muni_count = 0
    merged_count = 0
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
            merged_count += 1
        else: # If it is a muni wr, then
            new_muni_count += 1
            waterrightid_str = merged_row_list[waterrightid_ndx]
            if waterrightid_str not in merged_muni:
                merged_muni[waterrightid_str] = [merged_row_list] 
            else:
                (merged_muni[waterrightid_str]).append(merged_row_list)
        new_count += 1
   
    # Now process the muni water rights.
    # If there are corresponding water right IDs in the old file, copy their UGB and 
    # priority year data into the new UGB and CW3M_YEAR fields.
    # While working through the old file, as artificial water rights are encountered,
    # copy the muni artificial water rights to the dictionary and write the non-muni
    # artificial rights directly to the merged file.
    print("Beginning to merge old and new water rights now...")
    pouid_ndx = header_list.index("POUID")
    ugb_ndx = header_list.index("UGB")
    cw3m_year_ndx = header_list.index("CW3M_YEAR")
    old_count = 0
    old_muni_count = 0
    count_of_common_muni_in_ugb = 0
    artificial_count = 0
    for row in old_reader:
        old_count += 1
        if int(row[usecode_ndx]) == WRU_MUNICIPAL:
            old_muni_count += 1
        old_waterrightid_str = row[waterrightid_ndx]
        old_waterrightid_int = int(old_waterrightid_str)
        if old_waterrightid_str in merged_muni:
            if not (int(row[usecode_ndx]) == WRU_MUNICIPAL):
                print("old_waterrightid_str =", old_waterrightid_str, " row[usecode_ndx] =", row[usecode_ndx])
                print(f"{old_waterrightid_str} is for muni use in the new file.")
            pouid = int(row[pouid_ndx])
            if (pouid < 0):
                ugb = -pouid
                count_of_common_muni_in_ugb += 1
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
            artificial_count += 1
            merged_pod_record = row
            pouid = int(merged_pod_record[pouid_ndx])
            if (pouid < 0):
                ugb = -pouid
                count_of_common_muni_in_ugb += 1
            else:
                ugb = 0
            merged_pod_record.append(ugb) 
            merged_pod_record.append(merged_pod_record[year_ndx])
            merged_writer.writerow(merged_pod_record) 
            merged_count += 1

    # Now add the merged muni water rights back into the merged_file.
    print("merged_muni:")
    merged_muni_keys = merged_muni.keys()
    for key in merged_muni_keys:
        wr_id_list = merged_muni[key]
#        print("wr_id:", key)
        for row in wr_id_list:
#            print("  ", row)
            merged_writer.writerow(row)
            merged_count += 1
 
    print(f"There are {old_count} data rows in {old_pods_file_name}")
    print(f"    of which {artificial_count} are artificial")
    print(f"    and {old_muni_count} are for municipal use")
    print(f"There are {new_count} data rows in {new_pods_file_name}")
    print(f"    of which {new_muni_count} are for municipal use")
    print(f"There are {merged_count} data rows in {merged_pods_file_name}")
    print(f"    of which {count_of_common_muni_in_ugb} are for municipal use in urban growth areas")
    return new_count


if __name__ == "__main__":
    main()
