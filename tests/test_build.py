import os

def test_build():
    # Check if the server executable exists
    server_executable = "build/Release/OrderEngine.exe"
    assert os.path.isfile(server_executable), f"{server_executable} does not exist."

if __name__ == "__main__":
    test_build()
    print("Build verification passed.")
