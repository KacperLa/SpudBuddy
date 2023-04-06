import zmq

class zmq_req_rep_socket:
    def set_up(self):
        self.socket = self.context.socket(zmq.REQ)
        self.socket.connect(self.port)
        print("Socket %s Connected." %self.port)

    def __init__(self, context, port):
        self.port = port
        self.context = context
        self.attempts = 5
        self.busy = False
        self.set_up()
        
    def query(self, request, timeout=1500):
        success = False
        message = None
        if not self.busy:
            self.busy = True
            response = None
            self.socket.send_json(request)        
            for attempt in range(self.attempts):
                if (self.socket.poll(timeout) & zmq.POLLIN) != 0:
                    response = self.socket.recv_json()
                    if response is not None:
                        message = response['message']
                        success = response['success']
                    break
                print("No response")
            if response is None:
                print("No response from service rebuiling socket.")
                self.tear_down()
                self.set_up()
                success = False
                message = {"message": "This service is not responding."}
            self.busy = False
        else:
            success = False
            message = "Socket is busy, please try again." 
        return success, message

    def tear_down(self):
        self.socket.setsockopt(zmq.LINGER, 0)
        self.socket.close()
        print("Socket %s closed." %self.port)