import sqlite3
import argparse
import pprint

QUERY_AGENTS = """
    SELECT e.AgentId,Spec,Prototype,ParentId,Lifetime,EnterTime,ExitTime
    FROM AgentEntry AS e
    LEFT JOIN AgentExit AS x ON x.SimId = e.SimId AND x.AgentId = e.AgentId
    ORDER BY EnterTime,Prototype,e.AgentId;
"""

QUERY_FLOW_TOT = """
    SELECT t.TransactionId,t.Time,t.SenderId,t.ReceiverId,t.Commodity,SUM(r.Quantity) AS Quantity
    FROM Transactions AS t
    INNER JOIN Resources AS r ON t.SimId = r.SimId AND t.ResourceId = r.ResourceId
    GROUP BY t.SenderId,t.ReceiverId,t.Time
    ORDER BY Time,SenderId,ReceiverId,Commodity,TransactionId;
"""

QUERY_FLOW_NUC = """
    SELECT t.TransactionId,t.Time,t.SenderId,t.ReceiverId,t.Commodity,c.NucId,SUM(r.Quantity*c.MassFrac) AS Quantity
    FROM Transactions AS t
    INNER JOIN Resources AS r ON t.SimId = r.SimId AND t.ResourceId = r.ResourceId
    INNER JOIN Compositions AS c ON c.SimId = r.SimId AND c.QualId = r.QualId
    GROUP BY t.SenderId,t.ReceiverId,t.Time,c.NucId
    ORDER BY Time,SenderId,ReceiverId,Commodity,TransactionId;
"""

def table_eq(conn1, conn2, tbl_name):
    return query_eq(conn1, conn2, 'SELECT * FROM {1}'.format(tbl_name))

def query_eq(conn1, conn2, query):
    prepare(conn1)
    c = conn1.cursor()
    c.execute(query)
    expect = c.fetchall()
    return conn2, query, expect

def print_query(conn, query):
    prepare(conn)
    c = conn.cursor()
    for row in c.execute(query):
        pprint.pprint(row)

def check_query(conn, query, expect):
    """
    expect is a list of lists (all ordered) of expected values from the query.
    query is a valid sql string for the given sqlite 3 db connection.
    """

    prepare(conn)
    c = conn.cursor()
    c.execute(query)
    rows = c.fetchall()

    errors = []

    if len(rows) != len(expect):
        s = 'wrong number of rows: got {1}, expected {2}'.format(len(rows), len(expect))
        errors.append(s)

    for i, row in enumerate(rows):
        if i >= len(expect):
            break
        exp_row = expect[i]

        if len(row) != len(exp_row):
            s = 'wrong number of cols: got {1}, expected {2}'.format(len(row), len(exp_row))
            errors.append(s)
            return errors

        col = -1 
        for val, exp_val in zip(row, exp_row):
            col += 1
            if val != exp_val:
                if isinstance(float, val) and abs(val - exp_val) / exp_val < 1e-12:
                    continue
                s = 'value NE - row {1} col {2}: got {3}, expected {4}'.format(
                        i, col, val, exp_val)
                errors.append(s)

    return errors

def prepare(conn):
    c = conn.cursor()
    cmds = [
		'CREATE TABLE IF NOT EXISTS AgentExit (SimId BLOB,AgentId INTEGER,ExitTime INTEGER);',
		'CREATE TABLE IF NOT EXISTS Compositions (SimId BLOB,QualId INTEGER,NucId INTEGER, MassFrac REAL);',
		'CREATE TABLE IF NOT EXISTS Products (SimId BLOB,QualId INTEGER,Quality TEXT);',
		'CREATE TABLE IF NOT EXISTS Resources (SimId INTEGER,ResourceId INTEGER,ObjId INTEGER,Type TEXT,TimeCreated INTEGER,Quantity REAL,Units TEXT,QualId INTEGER,Parent1 INTEGER,Parent2 INTEGER);',
		'CREATE TABLE IF NOT EXISTS ResCreators (SimId INTEGER,ResourceId INTEGER,AgentId INTEGER);',
		'CREATE TABLE IF NOT EXISTS Agents (SimId BLOB,AgentId INTEGER,Kind TEXT,Spec TEXT,Prototype TEXT,ParentId INTEGER,Lifetime INTEGER,EnterTime INTEGER,ExitTime INTEGER);',
		'CREATE TABLE IF NOT EXISTS Inventories (SimId BLOB,ResourceId INTEGER,AgentId INTEGER,StartTime INTEGER,EndTime INTEGER,QualId INTEGER,Quantity REAL);',
		'CREATE TABLE IF NOT EXISTS TimeList (SimId BLOB, Time INTEGER);',
		'CREATE TABLE IF NOT EXISTS Transactions (SimId BLOB, TransactionId INTEGER, SenderId INTEGER, ReceiverId INTEGER, ResourceId INTEGER, Commodity TEXT, Time INTEGER);',
            ]
    for cmd in cmds:
        c.execute(cmd)
                
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='check database')
    parser.add_argument('db', metavar='DB', type=str,
                         help='sqlite database file to query')
    parser.add_argument('query', metavar='QUERY', type=str, nargs='?',
                         help='SQL query to be executed')
    parser.add_argument('--compare', metavar='DB', type=str,
                         help='another database to compare query against')
    args = parser.parse_args()

    print(args.db)
    print(args.compare)
    print(args.query)
    conn = sqlite3.connect(args.db)

    queries = [args.query]
    if args.query is None:
        queries = [QUERY_AGENTS,QUERY_FLOW_TOT,QUERY_FLOW_NUC]

    if args.compare is None:
        for q in queries:
            print('============ Running Query ==============')
            print(q, '\n-----------------------------------------')
            print_query(conn, q)
    else:
        conn2 = sqlite3.connect(args.compare)
        for q in queries:
            print('============ Running Query ==============')
            print(q, '\n-----------------------------------------')
            errors = check_query(*query_eq(conn2, conn, q))
            for e in errors:
                print(e)
