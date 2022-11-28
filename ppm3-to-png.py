"""
Takes every ppm file in the current dir and all subdirs and makes a 
corresponding png file suffixed with '-generated.png'

Prints the path of the ppm files its working on to show progress.

Made as a work around bc ImageMagick wasn't working on my VM for some reason.
"""

from PIL import Image
Image.MAX_IMAGE_PIXELS = None
import glob
import os

for ppm_path in glob.iglob("**/*.ppm*", recursive=True):
    print(ppm_path + " -->")
    img = Image.open(ppm_path)
    png_path = ppm_path[:-4] + "-generated.png"
    new_img = img.copy()
    img.close()
    if os.path.exists(png_path):
        os.remove(png_path)
    new_img.save(png_path)
    print("\t" + png_path)
    new_img.close()