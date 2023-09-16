import socket
import msgpack

# Create a socket object
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Define the host and port to bind to
host = "0.0.0.0"  # Listen on all available network interfaces
port = 3000

# Bind the socket to the host and port
server_socket.bind((host, port))

# Listen for incoming connections (up to 5 clients in the queue)
server_socket.listen(5)

print("Server listening on {}:{}".format(host, port))

while True:
    # Accept incoming client connections
    client_socket, client_address = server_socket.accept()
    print("Accepted connection from {}:{}".format(client_address[0], client_address[1]))

    try:
        while True:
            # Receive data from the client
            data = client_socket.recv(1024)
            if not data:
                break  # No more data, exit the loop

            # Decode the data from binary to string using msgpack
            data = msgpack.unpackb(data, raw=False)
            # Print the data received
            print("Received data from client: {}".format(data))

            # Send a response back to the client (optional)
            # client_socket.send("Hello, client!".encode("utf-8"))
    except KeyboardInterrupt:
        break

    # Close the client socket
    client_socket.close()

# Close the server socket
server_socket.close()