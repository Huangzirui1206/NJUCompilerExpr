import os

parser = "./parser"
result = "../Result/"
test = "../Test/"


for dirpath, dirnames, filenames in os.walk(test):
    if filenames:
        for filename in filenames:
            targetname = result + filename.split('.')[0] + '.output'
            target = open(targetname, 'w')
            target.write(os.popen(parser + ' ' + test +filename).read())
            target.close()