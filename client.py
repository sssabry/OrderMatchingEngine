import socket
def send_order(order):
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect(('127.0.0.1', 54000))
    client_socket.sendall(order.encode())

    response = client_socket.recv(4096)
    print('Server response: ', response.decode())
    client_socket.close()

if __name__ == "__main__":
    while True:
        side = input("Enter side (0 for Buy, 1 for Sell): ")
        price = input("Enter price: ")
        quantity = input("Enter quantity: ")
        order = f"{side} {price} {quantity}"
        send_order(order)
