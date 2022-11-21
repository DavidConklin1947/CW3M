import shapefile
IDU_layer_file = "C:/CW3M.git/trunk/DataCW3M/WRB/IDU_WRB.shp"
IDU_layer = shapefile.Reader(IDU_layer_file)
num_idus = len(IDU_layer)
print("num_idus is", num_idus, "in", IDU_layer_file)
max_idu_id = -1
for i in range(num_idus):
	idu = IDU_layer.record(i)
	if int(idu['IDU_ID']) > max_idu_id:
		max_idu_id = int(idu['IDU_ID'])

print("max_idu_id is", max_idu_id, "in", IDU_layer_file)

