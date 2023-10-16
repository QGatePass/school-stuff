import sqlite3

con = sqlite3.connect('student.db')
cur = con.cursor()

courses = ['ece405','ece421','ece427','ece431','ele403','ele473','cve421','feg404']

for course in courses:
    cur.execute(f"""create table student{course} (reg_no text, fingerprint text)""")
    cur.execute(f"""create table register{course} (reg_no text, date text)""")
    con.commit()
cur.execute("""create table student_general (reg_no text, fingerprint text)""")
con.commit()
con.close()
print('Done!')