# CW3M Utility
12/3/22
#### Video Demo: <URL HERE>
#### Description
CW3M is a watershed simulation model written in C++ which uses ESRI shapefiles to represent the land and stream network in a spatially explicit way. This Python program allows the user/programmer to analyze and modify the large shapefiles used by CW3M. CW3M also uses CSV and XML files; this program is intended to be a framework to which more functions may be added from time to time which analyze and modify these other CW3M data files.

"CW3M" stands for Community Willamette Whole Watershed Model.  It represents the Willamette River basin in western Oregon as a map tiled by about 200,000 "IDUs", polygons of varying shapes and sizes.  An IDU ("Independent Decision Unit") has several hundred attributes. Some of the attribute values are expected to preserve certain relationships across the set of IDUs. For example, a unique IDU_ID attribute value should be assigned to each tile on the map; there should be no duplicate IDU_ID values.

Occasionally, new tilings of the map are developed, to improve the representation of attributes of interest for a particular study. By convention, IDU tiles which remain unchanged keep their IDU_ID values, and new tiles get unique new IDU_ID values. The first use of this Python program is to screen a new IDU shapefile for duplicate IDU_IDs and assign replacement IDU_ID values which are unique.

Three other IDU attributes pertain to the LULC ("land use, land cover") classification of the IDU polygon. Those attributes are named LULC_A, LULC_B, and VEGCLASS. The classification scheme is hierarchical. LULC_A is the most general, with only a small number of classes, such as forest, agricultural, developed, and water body. VEGCLASS (a.k.a. LULC_C) is the most specific, with hundreds of classes. Each VEGCLASS category is associated with an LULC_B category, and each LULC_B category is associated with an LULC_A category. The second use of this Python program is to scan the shapefile for consistency with a LULC hierarchy specified in a separate XML file and, where possible, supply missing values.

For this program, at least initially, the Python programmer and the user are the same person. The program is written with the expectation that other potential uses will be identified and subsequently implemented with additional code.

#### How to use the program

To start the program, type "python project.py" at the command line. Invoking the program without arguments will cause it to display usage information.  Input data files are assumed to be located in the current directory. New data files created by the program are placed in a subdirectory "NewFiles" of the current directory.

The --do command line flag is required in order to tell the program what task to perform. As of the date under the title above, two functions ("--do basin" and "--do idu") are implemented and two more are under development ("--do lulc" and --do wr").

##### Command line flags
"--attribute" identifies the attribute of interest

"--do" specifies the task to be performed

"--idu" identifies the IDU shapefile

"--lulc" identifies LULC XML file

"--next_id" sets the value of FIRST_NEW_IDU_ID

"--value_of_attribute" specifies the value of the attribute of interest

"--wr" identifies the water right point-of-diversion file

##### --attribute name
name = the name of the attribute to be used for the extraction task, a string. Defaults to VEGCLASS.

##### --do extract | idu_id | lulc | wr
"--do extract" tells the program to extract the shapefile records having a specified value of a specified attribute. The name and value of the attribute are defined by the --attribute and --value_of_attribute flags.

"--do idu_id" tells the program to check the IDU layer for duplicate IDU_ID values and replace them with unique values. This function requires the IDU shapefile to be specified with the --idu flag. The IDU layer is modified in place.

"--do lulc" tells the program to check the LULC_A, LULC_B, and VEGCLASS attribute values in the IDU layer for consistency with the LULC specification, and make repairs to the extent possible. This function requires both the IDU shapefile and the LULC XML file to be specified, with the --idu and --lulc flags. The modified IDU layer has the same name as the input IDU layer, but is placed in the NewFiles subdirectory. The LULC file is not modified.

"--do wr" tells the program to check the internal consistency of the water rights data CSV files and make changes as appropriate. New files are created for the modified versions of the input files. The new files have the same names as the input file, but are placed in the NewFiles subdirectory.

##### --idu IDU_layer
IDU_layer = path and base name of the IDU shapefile. The IDU file will be modified in place when appropriate by changing the values of attributes. The program does not change the number or shape of the IDU polygons.

##### --lulc LULC_spec
LULC_spec = path and file name of the XML file which specifies the LULC hierarchy. The file name is usually "LULC.xml". This file is used only as input; the program doesn't change it.

##### --next_id idu_id_value
idu_id_value = the next unused idu_id value; replaces the default value (164892) of FIRST_NEW_IDU_ID. The default value is valid for IDU_WRB.shp in CW3M 1.2.3.

##### --value_of_attribute value
value = the value of the attribute to be used for the extraction task, an integer. Defaults to the VEGCLASS value for shrubland.

##### --wr POD_file
POD_file = path and file name of the CSV file containing the points of diversion. The program expects the substring "pod" to be part of the file name, case-insensitively. The program expects the corresponding points-of-use file to have a similar name, with the substring "pou" in place of "pod". The names of these files are usually "wr_pods.csv" and "wr_pous.csv". Input POD and POU files are treated as read-only. When modified versions are made, their file names are the same as the names of the input files, but they are placed in the NewFiles subdirectory.

#### Resources used by the program
The program uses the "pyshp" and "argparse" library modules, loaded with the "pip install" command.

#### Program structure and functions for Pytest
There is a separate function for each task: do_extract(), do_idu(), do_lulc(), and do_wr(). The test_project.py file has corresponding functions: test_do_extract(), test_do_idu(), and so on. Dat for the test functions is drawn from the CW3M 1.2.3 release in April, 2022, and from development work going on now for simulations of wetlands in the McKenzie River basin.

The do_extract() function returns the number of IDUs extracted. The test_do_extract() function asserts that there are only two IDUs in the IDU_BLU shapefile with shrubland as the value of their VEGCLASS attribute.

The do_idu_id() function returns the number of duplicate IDU_ID values. The test_do_idu() function asserts that 78 duplicate IDU_ID values are found in the IDU_BLU shapefile.

The do_lulc() function returns the number of IDUs for which LULC_A/LULC_B/VEGCLASS values were changed.

The do_wr() function returns the number of data records in the new POD file. The test_do_wr() function asserts that do_wr("wr_pods.csv") returns 22057, the number of data records in the wr_pods.csv file in CW3M 1.2.3.