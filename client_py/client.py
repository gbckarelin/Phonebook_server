import socket
import json
import time

HOST = '127.0.0.1'  # Server address
PORT = 8080  # Server port

def connect_to_server(host, port):
    print("Connecting to the server...")
    while True:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            try:
                s.connect((host, port))
                print("Connected to the server successfully.")
                break
            except ConnectionRefusedError:
                print("Failed to connect to the server. Retrying in 5 seconds...")
                time.sleep(5)

def send_json_data(data):
    json_data = json.dumps(data)
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        try:
            s.connect((HOST, PORT))
            s.sendall(json_data.encode())
            response = s.recv(1024)
            if response:
                try:
                    pretty_json = json.loads(response.decode())
                    print("")
                    print(json.dumps(pretty_json, indent=4), "\n")
                except json.decoder.JSONDecodeError:
                    print( response.decode(),"\n")
        except ConnectionRefusedError:
            print("Failed to connect to the server.","\n")

def add_contact(username):
    nickname = input("Enter nickname: ")
    phone_number = input("Enter phone number: ")
    first_name = input("Enter first name: ")
    last_name = input("Enter last name: ")
    middle_name = input("Enter middle name: ")
    note = input("Enter note: ")

    contact_data = {
        "username": username,
        "action": "add_contact",
        "nickname": nickname,
        "phone_number": phone_number,
        "info": {
            "first_name": first_name,
            "last_name": last_name,
            "middle_name": middle_name,
            "note": note
        }
    }

    send_json_data(contact_data)

def delete_contact(username):
    print("Enter the contact you want to delete.")
    delete_choice = None
    while delete_choice not in {"P", "N"}:
        delete_choice = input("Enter [P] if you want to input a phone number \n   or [N] if you want to input nickname: ")
        if delete_choice == "P":
            phone_number = input("Enter phone number: ")
        elif delete_choice == "N":
            nickname = input("Enter nickname: ")
        else:
            print("Enter a valid option")

    contact_data = {
        "username": username,
        "action": "delete_contact",
        "delete_choice": delete_choice,
        "phone_number": phone_number if delete_choice == "P" else None,
        "nickname": nickname if delete_choice == "N" else None,
    }

    send_json_data(contact_data)

def find_contact(username):
    search = input("Enter a search term to find a contact: ")
    contact_data = {
        "username": username,
        "action": "find_contact",
        "search_term": search
    }
    send_json_data(contact_data)

def listig_contacts(username):
    contact_data = {
        "username": username,
        "action": "listing_contacts"
    }
    send_json_data(contact_data)


def show_menu():
    print("======== MENU =======")
    print("Choose an action:")
    print("1. Add a new contact")
    print("2. Delete a contact")
    print("3. Find a contact")
    print("4. Show all contacts")
    print("====================\n")
    choice = input("Enter your choice: ")
    return choice.strip()

if __name__ == "__main__":
    connect_to_server(HOST, PORT)
    username = input("Enter your username: ")
    while True:
        choice = show_menu()
        if choice == '5':
            print('Exiting...')
            break
        elif choice == '1':
            add_contact(username)
        elif choice == '2':
            delete_contact(username)
        elif choice == '3':
            find_contact(username)
        elif choice == '4':
            listig_contacts(username)
        else:
            print("Enter a valid option")
