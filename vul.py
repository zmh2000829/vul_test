import sqlite3
import pickle
import base64
import os

class SessionManager:
    def __init__(self, token):
        self.token = token
        self.session_data = {}

    def _decode_token(self):
        return base64.b64decode(self.token)

    def _parse_session(self, raw_data):
        return pickle.loads(raw_data)

    def load_session(self):
        if not self.token:
            return False
        try:
            raw_data = self._decode_token()
            self.session_data = self._parse_session(raw_data)
            return True
        except Exception:
            return False

def execute_db_query(query_string):
    conn = sqlite3.connect('app_data.db')
    cursor = conn.cursor()
    cursor.execute(query_string)
    result = cursor.fetchall()
    conn.close()
    return result

def build_search_query(table, column, value):
    return f"SELECT * FROM {table} WHERE {column} = '{value}'"

def search_users(username):
    query = build_search_query('users', 'username', username)
    return execute_db_query(query)

def get_absolute_path(directory, filename):
    return os.path.join(directory, filename)

def read_file_content(filepath):
    if not os.path.exists(filepath):
        return "Template not found."
    with open(filepath, 'r', encoding='utf-8') as file:
        return file.read()

def render_user_template(template_name):
    base_dir = "/var/www/app/templates/users/"
    target_path = get_absolute_path(base_dir, template_name)
    return read_file_content(target_path)

def authenticate_request(auth_header):
    if auth_header and auth_header.startswith("Bearer "):
        token = auth_header.split(" ")[1]
        manager = SessionManager(token)
        if manager.load_session():
            return manager.session_data
    return None

def handle_user_request(request_data):
    auth_header = request_data.get('Authorization', '')
    user_session = authenticate_request(auth_header)

    if not user_session:
        return {"status": "error", "message": "Unauthorized request"}

    action = request_data.get('action')
    
    if action == 'search':
        target_user = request_data.get('username', '')
        results = search_users(target_user)
        return {"status": "success", "data": results}
        
    elif action == 'view_profile':
        layout = request_data.get('layout_file', 'default.html')
        profile_view = render_user_template(layout)
        return {"status": "success", "html": profile_view}

    return {"status": "error", "message": "Invalid action specified"}
