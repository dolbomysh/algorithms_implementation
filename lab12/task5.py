import psycopg2

class DBConnection:
    def __init__(self, dbname, user, password, host='localhost', port=5432):
        try:
            self.conn = psycopg2.connect( dbname=dbname, user=user, password=password, host=host, port=port)
            self.cursor = self.conn.cursor()
        except:
            print('Can`t establish connection to database')

    def set_session_parameters(self, work_mem, maintenance_work_mem, random_page_cost):
        self.cursor.execute(f"SET work_mem TO '{work_mem}';")
        self.cursor.execute(f"SET maintenance_work_mem TO '{maintenance_work_mem}';")
        self.cursor.execute(f"SET random_page_cost TO {random_page_cost};")

    def execute_query(self, query, fetch=False):
        try:
            self.cursor.execute(query)
            self.conn.commit()
            if fetch:
                return self.cursor.fetchall()
        except Exception as exception:
            print(f"Error executing query: {exception}")
            self.conn.rollback()

    def explain_query(self, query):
        explain_query = f"EXPLAIN ANALYZE {query}"
        return self.execute_query(explain_query, fetch=True)

    def close(self):
        self.cursor.close()
        self.conn.close()

# Пример выполнения запроса

db = DBConnection(dbname="postgres", user="postgres", password="secret")

db.set_session_parameters(work_mem="64MB", maintenance_work_mem="64MB", random_page_cost=1.5)

db.execute_query(""" CREATE TABLE users (
                    id SERIAL PRIMARY KEY,
                    name TEXT NOT NULL); """)

db.execute_query("""INSERT INTO users (name) VALUES 
                   ('Alice'), ('Bob'), ('Charlie');""")


query = "SELECT * FROM users;"

result = db.execute_query(query, True)

for line in result:
    print(line)

explain_plan = db.explain_query(query)

print("\nQuery Execution Plan:")
for line in explain_plan:
    print(line)

db.close()

# Вывод
# (1, 'Alice')
# (2, 'Bob')
# (3, 'Charlie')
#
# Query Execution Plan:
# ('Seq Scan on users  (cost=0.00..22.70 rows=1270 width=36) (actual time=0.003..0.004 rows=3 loops=1)',)
# ('Planning Time: 0.026 ms',)
# ('Execution Time: 0.011 ms',)
