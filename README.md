# SmartContour

## GOAL: 
A program to interactively segment organs in a slice-by-slice fashion, one organ at a time

## INPUT: 
two possibilities
       - a DICOM dataset
       - a .raw volume file with an .nhdr description file (header for the raw)

## OUTPUT: 
a .raw volume file with the segmentation mask and the respective .nhdr description file

## To run:
- try to run the .exe files in SmartContour\bin

## To compile:
- there is a VS2010_project folder that contains the respective Visual Studio solution files
- open the *.sln file
- verify the includes and links in the project.
- new executable files will be created in <project_name>\bin


## Step-by-step for the included dataset (dataset 1)
1. run SmartContour.exe
2. click File->Open Images/Raw
3. select the .nhdr file in folder dataset 1 and click open (it may take a few seconds to load)
4. in Navigation, change the density window and slice until you find the organ to be segmented. Use PAN to pan (left-drag) and zoom (right-drag)
5. next step is to create a contour. To do that, click ADD/Remove point. A message "DEFINE THE WORKSPACE" appears at the bottom of the screen.
6. click and slide to define the WORKSPACE area. Make sure that the target organ fits completely in this area for all slices. Hit enter to confirm selection. It takes a few seconds to preprocess the WORKSPACE, depending on size.
7. By default the WORKSPACE is not highlighted. Click "Show Workspace" to toggle highlight of the WORKSPACE.
8. Navigate to the first slice of the target organ to start selecting points.
9. Click and release anywhere on the border of the organ to add a point. 
10. Move the cursor along the border. The system will try to fit a red curve along the border.
11. Click again to add more points and to consolidate the curve. Right click to undo the last added point.
12. Select Edit Point in the menu to edit the added points. Click and drag to move the points.
13. Continue adding and editing until you make sure that the whole border has been covered.
14. Try the Expand White/Blue Points options to automatically fit an existing curve.
15. Click Close Contour when you are satisfied.
16. Save points regularly as the program may crash abruptly. You can then load the points and continue later.
17. Before going to the next slice, you can click Copy Contour to use it in the next slice
18. Move to the next slice and Paste Contour (try Expand points), or start a new one using ADD/Remove Point.  
19. Repeat for all slices.
20. Click Save Contour to export the segmentation mask.


## Step-by-step for other DICOM dataset
1. run SmartContour.exe
2. click File->Convert Images/DICOM
3. the system will process for a few minutes to make this conversion (no progress bar, sorry)
4. go to the step 2 of the step-by-step above and continue until it's done.

## Limitations and known bugs:
- Crashes when applying filter to an already filtered dataset 

## OBS
We used Windows 7 64bit and MS Visual Studio 2010. 
A shader supported graphics card is necessary. We used an Nvidia Geforce 9600GT, but many other models should work fine. Binaries last tested in a Core i7 and nVidia GTX 1050

Good luck! :-)