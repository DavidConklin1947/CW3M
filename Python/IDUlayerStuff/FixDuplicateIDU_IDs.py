import shapefile
import os.path

IDU_layer_file = "C:/CW3M.git/trunk/DataCW3M/McKenzie/IDU_McKenzie.shp"
# IDU_layer_file = "C:/CW3M.git/trunk/DataCW3M/McKenzie/Richey/IDU_McKenzie.shp"
first_new_idu_id = 164892
path_for_new_files = "C:/CW3M.git/trunk/DataCW3M/McKenzie/"  + "new_shapefiles/"

path, base_name = os.path.split(IDU_layer_file)
new_pathname = path_for_new_files + base_name
print(f"new pathname is {new_pathname}")

IDU_layer = shapefile.Reader(IDU_layer_file)
num_idus = len(IDU_layer)
print("num_idus is", num_idus, "in", IDU_layer_file)

num_zero_area_idus = 0
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

max_idu_id = -1
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
print("\nwait for it...")

num_duplicates = 0
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
	dup_found = False
	j = i + 1
	while j < num_idus:
		idu_j = IDU_layer.record(j)
		idu_id_j = idu_j.IDU_ID
		if idu_id_i != idu_id_j:
			if not dup_found:
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

print("max_idu_id is", max_idu_id, "in", IDU_layer_file)
print("first_new_idu_id is", first_new_idu_id)
print('')
print("num_idus is", num_idus, "in", IDU_layer_file)
print("num_non_neg_fid_wetlan is", num_non_neg_fid_wetlan)
print("num_duplicates =", num_duplicates)

print(f"beginning to create new shapefile: {new_pathname}")
writer = shapefile.Writer(new_pathname)
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
