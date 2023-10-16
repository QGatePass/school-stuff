import RPi.GPIO as GPIO
import os
from rpi_lcd import LCD
from time import sleep
from KBLIB import keypad
from datetime import date
import random
import sqlite3
import fingerprint_simpletest_rpi as fgn


screen = LCD()
key = keypad()
pswd = '4050'
this_course = ""

courses = ['ece405','ece421','ece427','ece431','ele403','ele473','cve421','feg404']


def info():
    screen.text('ECE Attendance Monitor',1)
    screen.text('Charles Oranugo',2)
    screen.text('Made By: 2019364XXX set',3)
    screen.text('Nwora Chidubem',4)
    sleep(5)
    screen.clear()

def main_menu():
    global this_course
    screen.clear()
    screen.text('1. Admin Menu',1)
    screen.text('2. Courses',2)
    screen.text('3. Shutdown',3)
    choice = key.readKeypad()
    if choice == '1':
        sleep(1)
        admin_menu()
    elif choice == '2':
        sleep(1)
        courses()
    elif choice == '3':
        sleep(1)
        shutdown()
    else: 
        screen.clear()
        screen.text('Not a Valid Option!',2)
        sleep(2)
        main_menu()

def admin_menu():
    global this_course
    screen.clear()
    screen.text('Enter Password:',1)
    sleep(.5)
    pword = key.readKeypad()
    screen.text(pword,2)
    if pswd == pword:
        admin_choice()
    else:
        screen.clear()
        screen.text(f'Wrong Password {pword}',2)
        sleep(1)
        main_menu()

def shutdown():
    screen.clear()
    screen.text('Saving Files...',1)
    sleep(1)
    screen.text('Closing Apps...',2)
    sleep(1)
    screen.text('Fucking around...',3)
    sleep(1)
    screen.clear()
    screen.text('Flip Power switch in 1 min',2)
    sleep(1)
    os.system('sudo shutdown -h now')
        
def enroll_student():
    """ This function enrolls students into the database """
    global this_course
    print(this_course)
    screen.clear()
    try:
        con = sqlite3.connect('student.db')
        cur = con.cursor()
        screen.clear()
        cur.execute(f"""select count(reg_no) from student_general""")
        result = cur.fetchall()
        temp_no = int(result[0][0])
        screen.text('Enter Reg No: ',2)
        sleep(.5)
        reg_no = key.readKeypad()
        screen.text(f'{reg_no}',3)
        sleep(2)
        screen.clear()
        screen.text(reg_no,1)
        screen.text('1. Procced',2)
        screen.text('2. Cancel',3)
        sleep(.5)
        choice = key.readKeypad()
        if choice == '1':
            cur.execute(f"""select * from student_general where reg_no = ?""",[reg_no])
            result = cur.fetchall()
            if len(result) == 0:
                try:
                    valid = fgn.enroll_finger(temp_no)
                    if valid:
                        cur.execute(f"""insert into student_general values(?,?)""",[reg_no,temp_no])
                        con.commit()
                        con.close()
                        screen.clear()
                        screen.text(f'{reg_no} registered on general database!',2)
                        sleep(3)
                        screen.clear()
                        screen.text('1. Continue ',1)
                        screen.text('2. Main Menu',2)
                        sleep(.5)
                        choice = key.readKeypad()
                        if choice == '1':
                            enroll_student()
                        elif choice == '2':
                            main_menu()
                        else:
                            screen.clear()
                            screen.text(f'Invalid Option {choice}',2)
                            sleep(2)
                            main_menu()
                    else:
                        screen.clear()
                        screen.text('Fingerprint not captured!',2)
                        sleep(2)
                except Exception as e:
                    print(e)
                    screen.clear()
                    screen.text('Error occurred', 2)
                    sleep(1)
                    main_menu()
            else:
                screen.clear()
                screen.text(f'{reg_no} already registered!',2)
                sleep(1)
                con.close()
        else:
            enroll_student()        
    except Exception as e:
        print(e)
        screen.clear()
        screen.text('Error occurred', 2)
        sleep(1)
        admin_choice()

def add_to_course():
    """This function adds a student from the database to a course"""
    global this_course
    course_list()
    screen.clear()
    screen.text('Enter student Reg No',2)
    sleep(.5)
    reg_no = key.readKeypad()
    screen.text(reg_no,3)
    sleep(2)
    screen.clear()
    screen.text(reg_no,1)
    screen.text('1. Procced',2)
    screen.text('2. Cancel',3)
    sleep(.5)
    choice = key.readKeypad()
    if choice == '1':
        con = sqlite3.connect('student.db')
        cur = con.cursor()
        cur.execute("""select * from student_general where reg_no = ?""",[reg_no])      #check if student in general database
        result = cur.fetchall()
        if len(result) > 0:
            cur.execute(f"""select * from student{this_course} where reg_no = ?""",[reg_no])        #check if student already on this_course
            res = cur.fetchall()
            if len(res) == 0:
                cur.execute(f"""insert into student{this_course} values(?,?)""",[result[0][0], result[0][1]])
                con.commit()
                screen.clear()
                screen.text(f'{reg_no} registered on {this_course} database!',2)
                sleep(2)

            else :
                screen.clear()
                screen.text(f'{reg_no} already registered!',2)
                sleep(2)
                
        else :
            screen.clear()
            screen.text(f'{reg_no} not in general database!',2)
            sleep(2)
    else:
        admin_choice()
    screen.clear()
    screen.text('1. Continue ',1)
    screen.text('2. Back', 2)
    screen.text('3. Main Menu',3)
    sleep(.5)
    choice = key.readKeypad()
    if choice == '1':
        add_to_course()
    elif choice == '2':
        admin_choice()
    elif choice == '3':
            main_menu()

       
def admin_choice():
    """This function is called when the user enters the admin option."""
    global this_course
    screen.clear()
    screen.text('1. Enroll Student',1)
    screen.text('2. Add to Course',2)
    screen.text('3. Send Records',3)
    screen.text('4. Main Menu',4)
    sleep(.5)
    ad_option = key.readKeypad()
    if ad_option == '1':
        enroll_student()
        screen.clear()
        screen.text('1. Continue ',1)
        screen.text('2. Back', 2)
        screen.text('3. Main Menu',3)
        sleep(.5)
        choice = key.readKeypad()
        if choice == '1':
            enroll_student()
        elif choice == '2':
            admin_choice()
        elif choice == '3':
            main_menu()
        else:
            screen.clear()
            screen.text(f'Invalid Option {choice}',2)
            sleep(2)
            main_menu()
    elif ad_option == '2':
        add_to_course()
        screen.clear()
        screen.text('1. Continue ',1)
        screen.text('2. Main Menu',2)
        choice = key.readKeypad()
        if choice == '1':
            add_to_course()
        elif choice == '2':
            main_menu()
        else:
            screen.clear()
            screen.text(f'Invalid Option {choice}',2)
            sleep(2)
            main_menu()
    elif ad_option ==  '3':
        screen.clear()
        screen.text('Attendance records sent!',2)
        sleep(2)
        screen.clear()
        screen.text('1. Continue ',1)
        screen.text('2. Main Menu',2)
        choice = key.readKeypad()
        if choice == '1':
            admin_choice()
        elif choice == '2':
            main_menu()
        else:
            screen.clear()
            screen.text(f'Invalid Option {choice}',2)
            sleep(2)
            main_menu()
    elif ad_option ==  '4':
        main_menu()
    else:
        screen.clear()
        screen.text('Invalid Option',2)
        sleep(2)
        admin_choice()

       
def course_list():
    """This function is used whenever there is need to select the course. Calls course_2()."""
    global this_course
    screen.clear()
    screen.text('1. ECE427  2. ECE405',1)
    screen.text('3. ECE421  4. ECE431 ',2)
    screen.text('5. ELE403  6. ELE473',3)
    screen.text('7. Next',4)
    clist = {'1':'ece427','2':'ece405','3':'ece421','4':'ece431', '5':'ele403', '6':'ele473','7':'cve421','8':'feg404'}
    sleep(.5)
    choice = key.readKeypad()
    if choice == '7' :
        screen.clear()
        screen.text('7. CVE421  8. FEG404',1)
        screen.text('9. Previous',2)
        sleep(.5)
        choice = key.readKeypad()
        this_course = clist.get(choice,'Invalid')
        if choice not in ['7','8','9'] :
            screen.clear()
            screen.text('Invalid Input!',2)
            sleep(1)
            course_list()
        if choice == 9:
            course_list()
    else :
        this_course = clist.get(choice,'Invalid')
    if choice == '9' and this_course == 'Invalid':
        course_list()
    elif choice != '9' and this_course == 'Invalid':
        screen.clear()
        screen.text('Select a course',2)
        screen.text(f'Current {this_course}, {choice}',3)
        sleep(3)
        course_list()
    screen.clear()
    screen.text(this_course,2)
    sleep(2)

def courses():
    global this_course
    course_list()
    screen.clear()
    screen.text('Password:',2)
    sleep(.5)
    pword = key.readKeypad()
    if pword == pswd:
        course_options()
    else:
        screen.clear()
        screen.text(f'Wrong Password {pword}',2)
        sleep(2)
        main_menu()
    
        
def course_options():
    global this_course
    screen.clear()
    screen.text('1. Sign In',1)
    screen.text('2. End',2)
    sleep(.5)
    choice = key.readKeypad()
    if choice == '1':
        try:
            found = fgn.get_fingerprint()
            if found:
                con = sqlite3.connect('student.db')
                cur = con.cursor()
                cur.execute(f"""select * from student{this_course} where fingerprint = ?""",[str(fgn.finger.finger_id)])
                result = cur.fetchall()
                if len(result) > 0:
                    reg_no = result[0][0]
                    timestap = date.today().strftime("%d-%m-%y")
                    cur.execute(f"""insert into register{this_course} values (?,?)""",[reg_no,timestap])
                    con.commit()
                    screen.clear()
                    screen.text(f'{reg_no} signed!',2)
                    con.close()
                    sleep(2)
                    course_options()
            else:
                screen.clear()
                screen.text(f'Student not found in course database. Register!',2)
                sleep(3)
                course_options()
        except Exception as e:
            print(e)
            screen.clear()
            screen.text('Error occurred', 2)
            sleep(1)
            main_menu()
    elif choice == '2':
        main_menu()
    else:
        screen.clear()
        screen.text('Please enter a valid option',2)
        course_options()



info()
main_menu()
