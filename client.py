import socket
import tkinter as tk
from tkinter import messagebox
from threading import Thread

def send_order(order):
    try:
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client_socket.connect(('127.0.0.1', 60000))
        client_socket.sendall(order.encode())

        response = client_socket.recv(4096)
        client_socket.close()

        return response.decode()
    except ConnectionRefusedError:
        return "Error: Connection refused. Ensure the server is running and accessible."
    except ConnectionResetError:
        return "Error: Connection was reset by the server. The server might have closed the connection unexpectedly."
    except socket.error as e:
        return f"Socket error: {e}"
    except Exception as e:
        return f"Unexpected error: {e}"

# Function to receive updates from the server
def receive_updates(update_callback):
    try:
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client_socket.connect(('127.0.0.1', 60000))

        while True:
            data = client_socket.recv(4096)
            if data:
                update_callback(data.decode())
            else:
                break

        client_socket.close()
    except ConnectionRefusedError:
        update_callback("Error: Connection refused. Ensure the server is running and accessible.")
    except ConnectionResetError:
        update_callback("Error: Connection was reset by the server. The server might have closed the connection unexpectedly.")
    except socket.error as e:
        update_callback(f"Socket error: {e}")
    except Exception as e:
        update_callback(f"Unexpected error: {e}")

# Main application class for the GUI
class OrderBookClient(tk.Tk):
    def __init__(self):
        super().__init__()

        self.title("Order Book Client")
        self.geometry("400x300")

        # Inputs
        self.side_label = tk.Label(self, text="Enter side (0 for Buy, 1 for Sell):")
        self.side_label.pack(pady=5)
        self.side_entry = tk.Entry(self)
        self.side_entry.pack(pady=5)

        self.price_label = tk.Label(self, text="Enter price:")
        self.price_label.pack(pady=5)
        self.price_entry = tk.Entry(self)
        self.price_entry.pack(pady=5)

        self.quantity_label = tk.Label(self, text="Enter quantity:")
        self.quantity_label.pack(pady=5)
        self.quantity_entry = tk.Entry(self)
        self.quantity_entry.pack(pady=5)

        self.submit_button = tk.Button(self, text="Submit Order", command=self.submit_order)
        self.submit_button.pack(pady=10)

        self.result_label = tk.Label(self, text="")
        self.result_label.pack(pady=5)

        # Live feed
        self.feed_label = tk.Label(self, text="Live Feed:")
        self.feed_label.pack(pady=5)
        self.feed_text = tk.Text(self, height=10, width=50)
        self.feed_text.pack(pady=5)

        # Start receiving updates in a separate thread
        self.update_thread = Thread(target=receive_updates, args=(self.update_feed,))
        self.update_thread.daemon = True
        self.update_thread.start()

    # Submit the order and display server response
    def submit_order(self):
        side = self.side_entry.get()
        price = self.price_entry.get()
        quantity = self.quantity_entry.get()

        if not side or not price or not quantity:
            messagebox.showerror("Input Error", "All fields must be filled out")
            return

        order = f"{side} {price} {quantity}"
        response = send_order(order)
        self.result_label.config(text=f"Server response: {response}")

    # Update the live feed with new messages
    def update_feed(self, message):
        self.feed_text.insert(tk.END, message + "\n")
        self.feed_text.yview(tk.END) 

if __name__ == "__main__":
    app = OrderBookClient()
    app.mainloop()
