import subprocess
import time
import requests

def test_client_server():
    # Start the server
    server_process = subprocess.Popen(["build/Release/OrderEngine.exe"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    # Allow some time for the server to start
    time.sleep(5)

    try:
        # Run the client
        client_process = subprocess.run(["python", "clients/client.py"], capture_output=True, text=True)
        
        # Check if the client run was successful
        if client_process.returncode != 0:
            raise Exception(f"Client failed with return code {client_process.returncode}\n{client_process.stderr}")

        print("Client output:\n", client_process.stdout)

    except Exception as e:
        print(e)
    finally:
        # Terminate the server process
        server_process.terminate()
        server_process.wait()

if __name__ == "__main__":
    test_client_server()
    print("Client-server communication test passed.")
