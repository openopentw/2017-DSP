import sys

# fn_result = './result.txt'
fn_result = sys.argv[1]
fn_ans = './testing_answer.txt'

with open(fn_result) as f:
    result = f.readlines()

with open(fn_ans) as f:
    ans = f.readlines()

correct = 0
for i,res in enumerate(result):
    if ans[i].strip() == res.split(' ')[0]:
        correct += 1

print(correct / 2500)
