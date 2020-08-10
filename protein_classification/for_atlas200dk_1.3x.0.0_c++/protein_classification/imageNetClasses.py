class_names = '''Mitochondria
Nucleus
Endoplasmic reticulum
Nuclear speckles
Plasma membrane
Nucleoplasm
Cytosol
Nucleoli
Vesicles
Golgi apparatus'''.split("\n")

imageNet_classes = {}
i = 0
for items in class_names:
    imageNet_classes[i] = items
    i += 1
