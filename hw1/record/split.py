from os import path

fn = './record_model_05.txt'
folder = './05'

with open(fn) as f:
    lines = f.readlines()

for i in range(len(lines)):
    line = lines[i]
    if line[0] == '=':
        fn_iter = '_'.join([line.split(' ')[1], line.split(' ')[2]])

        print(path.join(folder, fn_iter))
        with open(path.join(folder, fn_iter), 'w') as f:
            for _ in range(18):
                i += 1
                print(lines[i], end='', file=f)
