#import plotly.express as px

#with open("output.csv", 'r') as f:
#    data = f.read().splitlines()
#    x_data = [int(digit) for line_ind, line in enumerate(data) for digit_ind, digit in enumerate(line.split(',')) if digit_ind % 2 == 1 and digit_ind > 0 and line_ind > 0]
#    y_data = [int(digit) for line_ind, line in enumerate(data) for digit_ind, digit in enumerate(line.split(',')) if digit_ind % 2 == 0 and digit_ind > 0 and line_ind > 0]
#    # print(sorted(list(set(map(int, x_data)))), sorted(list(set(map(int, y_data)))))
#fig = px.scatter(x=sorted(set(x_data)), y=sorted(set(y_data)))
#fig.show()



#from matplotlib import pyplot as plt
#from celluloid import Camera
#import sys

#with open("out.csv", 'r') as f:
#    data = f.read().splitlines()
#clear_data = [line.split(',') for line in data]
#fig = plt.figure()
#camera = Camera(fig)
#print(sys.argv)
#for j in range(int(sys.argv[1])*3, int(sys.argv[2])*3, 3):
#    frame = clear_data[j:j+3]
#    print(frame)
#    plt.scatter([sublist[0] for sublist in frame], [sublist[1] for sublist in frame])
#    camera.snap()
#animation = camera.animate()
#plt.show()

# OR
# animation.save('test.mp4')

from matplotlib import pyplot as plt
from celluloid import Camera
from tqdm import tqdm

with open("raw.csv", 'r') as f:
    data = f.read().splitlines()
clear_data = [line.split(',') for line in data]
fig = plt.figure()
camera = Camera(fig)
x = [float(sublist[0]) for sublist in clear_data]
y = [float(sublist[1]) for sublist in clear_data]
for j in tqdm(range(0, len(clear_data), 50)):
    #print(frame)
    plt.scatter(x[j:j+3], y[j:j+3])
    camera.snap()
animation = camera.animate()
plt.show()