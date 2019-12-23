import random as rd
import os
# TODO play with accounts
tests_op = ["long", "med", "short"]
tests_kind = rd.choice(tests_op)
#random num of ATMs
atm_num = rd.randint(7 if tests_kind == "long" else 4 if tests_kind == "med" else 1, 10 if tests_kind == "long" else 6 if tests_kind == "med" else 3)
#random num of accounts
accounts_num = (3 * atm_num) if tests_kind == "long" else (2 * atm_num) if tests_kind == "med" else atm_num
#print(str(accounts_num), atm_num, tests_kind)
# randomize accounts
accounts = []
passwords = []

for i in range(0, accounts_num):
    accounts.append(str(rd.randint(100, 999)))
    passwords.append(str(rd.randint(0, 9999)).zfill(4))

# Directions creation
#####
# get the current script path.
here = os.path.dirname(os.path.realpath(__file__))
subdir = "tests_pack"
dir_path = here + "/" + subdir
# create your subdirectory
if not os.path.exists(dir_path):
    os.mkdir(os.path.join(here, subdir))

here = dir_path # move to subdir
test_dir = tests_kind + "_" + str(atm_num) + "_atms"
dir_path = dir_path + "/" + test_dir

# create new test dir
version = 1
new_dir_path = dir_path + "_v" + str(version)
while os.path.exists(new_dir_path):#find new name
    version += 1
    new_dir_path = dir_path + "_v" + str(version)

test_dir = test_dir + "_v" + str(version)
dir_path = dir_path + "_v" + str(version)
#here we got name that doesnt exists
print(test_dir)
os.mkdir(os.path.join(here, test_dir))

####################
# make traces
opList = ["O", "D", "W", "B", "L", "T"]
min_ops_per_atm = accounts_num * len(opList) #approximately every account gets most of operations
max_ops_per_atm = min_ops_per_atm * 2
cmd_string = "./Bank " + str(atm_num)
for i in range(atm_num): #run for every atm
    filename = "atm_" + str(i + 1)
    filepath = os.path.join(here, test_dir, filename)
    cmd_string = cmd_string + " " + subdir + "/" + test_dir + "/atm_" + str(i + 1)
    ops_fd = open(filepath, 'w+')
    num_ops = rd.randint(min_ops_per_atm, max_ops_per_atm)
    # start to random operations per atm
    for j in range(1, num_ops):
        acc_num = rd.randint(0, len(accounts) - 1)
        opcode = rd.choice(opList)

        corr_acc = rd.randint(0, atm_num)# random if put correct account or not
        corr_password = rd.randint(0, atm_num) # random if put correct password or not

        acc_to_write = accounts[acc_num] if (opcode == "O" or opcode != "O" and corr_acc > 0) else str(rd.randint(100, 999))
        password_to_write = passwords[acc_num] if (opcode == "O" or opcode != "O" and corr_password > 0) else str(rd.randint(0, 9999)).zfill(4)
        #thus, we get correct lines mostly, accuracy increases with more ATMs in order to test threads harder
        operation = opcode + " " + acc_to_write + " " + password_to_write # added common operation attributes: opcode, acc, password
        if opcode == "T":
            dst_acc_num = rd.randint(0, len(accounts) - 1)
            corr_dst_acc = rd.randint(0, atm_num)# random if put correct account or not
            dst_acc_to_write = accounts[dst_acc_num] if (corr_dst_acc > 0) else str(rd.randint(100, 999))
            operation = operation + " " + dst_acc_to_write
        if opcode in ["O", "D", "W", "T"]:
            amount = str(rd.randint(0, 10000))
            operation = operation + " " + amount
        ops_fd.write(operation + "\n")
    ops_fd.close()

#now, write full command to file to easier execution
filename = "command"
filepath = os.path.join(here, test_dir, filename)
cmd_fd = open(filepath, 'w+')
cmd_fd.write(cmd_string + "\n")
print(cmd_string)
cmd_fd.close()
#STOPPED THERE