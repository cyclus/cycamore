import sqlite3
import argparse

def table_eq(conn1, conn2, tbl_name):
    return query_eq(conn1, conn2, 'SELECT * FROM {1}'.format(tbl_name))

def query_eq(conn1, conn2, query):
    c = conn1.cursor()
    c.execute(query)
    expect = c.fetchall()
    return conn2, query, expect

def print_query(conn, query):
    c = conn.cursor()
    for row in c.execute(query):
        print(row)

def check_query(conn, query, expect):
    """
    expect is a list of lists (all ordered) of expected values from the query.
    query is a valid sql string for the given sqlite 3 db connection.
    """

    c = conn.cursor()
    c.execute(query)
    rows = c.fetchall()

    errors = []

    if len(row) != len(exp_row):
        s = 'wrong number of rows: got {1}, expected {2}'.format(len(row), len(exp_row))
        errors.append(s)

    for i, row in enumerate(rows):
        if i >= len(expect):
            break
        exp_row = expect[i]

        if len(row) != len(exp_row):
            s = 'wrong number of cols: got {1}, expected {2}'.format(len(row), len(exp_row))
            errors.append(s)
            return errors

        col = 0
        for val, exp_val in zip(row, exp_row):
            if val != exp_val:
                s = 'value NE - row {1} col {2}: got {3}, expected {4}'.format(
                        i, col, val, exp_val)
                errors.append(s)
            col += 1

    return errors
                
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='check database')
    parser.add_argument('db', metavar='DB', type=str,
                         help='sqlite database file to query')
    parser.add_argument('query', metavar='QUERY', type=str,
                         help='SQL query to be executed')
    parser.add_argument('--compare', metavar='DB', type=str,
                         help='another database to compare query against')
    args = parser.parse_args()

    print(args.db)
    print(args.compare)
    print(args.query)
    conn = sqlite3.connect(args.db)

    if args.compare is None:
        print_query(conn, args.query)
    else:
        conn2 = sqlite3.connect(args.compare)
        check_query(query_eq(conn2, conn, args.query))
