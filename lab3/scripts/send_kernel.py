#!/usr/bin/env python3

import sys
import os
import serial
import threading
import time
import queue
import struct

def usage():
	print("Usage: python3 send_kernel.py <tty_device> <kernel_image>")
	print("  <tty_device>: TTY device path (e.g., /dev/ttyUSB0)")
	print("  <kernel_image>: Path to kernel image file")
	sys.exit(1)

def calculate_checksum(file_path):
	"""Calculate checksum by summing all bytes as ASCII values"""
	checksum = 0
	with open(file_path, 'rb') as f:
		while True:
			byte = f.read(1)
			if not byte:
				break
			checksum += ord(byte)
	return checksum

def to_little_endian_4bytes(num):
	"""Convert number to little endian 4 bytes"""
	return struct.pack('<I', num)

class KernelSender:
	def __init__(self, tty_device, kernel_image):
		self.tty_device = tty_device
		self.kernel_image = kernel_image
		self.ser = None
		self.message_queue = queue.Queue()
		self.state = "waiting_for_init"
		self.running = True

		# Calculate kernel info
		self.kernel_size = os.path.getsize(kernel_image)
		self.checksum = calculate_checksum(kernel_image)

		print(f"Kernel size: {self.kernel_size} bytes")
		print(f"Kernel checksum: {self.checksum}")

	def reader_thread(self):
		"""Thread to continuously read from TTY"""
		while self.running:
			try:
				if self.ser and self.ser.in_waiting:
					line = self.ser.readline().decode('utf-8', errors='ignore').strip()
					if line:
						print(f"DEBUG RAW: '{line}'")
						self.message_queue.put(line)
				time.sleep(0.01)  # Small delay to prevent busy waiting
			except Exception as e:
				print(f"Reader error: {e}")
				break

	def writer_thread(self):
		"""Thread to handle writing logic based on received messages"""
		while self.running:
			try:
				# Wait for message with timeout
				try:
					line = self.message_queue.get(timeout=1)
				except queue.Empty:
					continue

				# Process message based on current state
				if "Frank bootloader init" in line:
					print("Bootloader detected, sending loadimg command...")
					self.ser.write(b"loadimg\n")
					self.state = "waiting_for_address_prompt"

				elif "Please input kernel load address (default: 0x80000):" in line:
					load_address = input("Enter kernel load address (press Enter for default 0x80000): ")
					if not load_address.strip():
						load_address = "0x80000"
					print(f"Using load address: {load_address}")
					self.ser.write(f"{load_address}\n".encode())
					self.state = "waiting_for_size_request"

				elif "Please send kernel size and checksum" in line:
					print(f"Sending kernel size ({self.kernel_size}) and checksum ({self.checksum})...")
					# Send size and checksum as little endian 4 bytes each
					size_bytes = to_little_endian_4bytes(self.kernel_size)
					checksum_bytes = to_little_endian_4bytes(self.checksum)
					self.ser.write(size_bytes + checksum_bytes)
					self.state = "waiting_for_image_request"

				elif "Please send kernel image from UART now..." in line:
					print("Sending kernel image data...")
					# Send kernel image
					with open(self.kernel_image, 'rb') as f:
						self.ser.write(f.read())
					print("Kernel image sent successfully!")
					self.state = "completed"
					break

				elif "Start Loading kernel image..." in line:
					print("Kernel loading started successfully!")

				elif "load_image Error" in line:
					print("Load error detected, retrying in 5 seconds...")
					time.sleep(5)
					self.ser.write(b"loadimg\n")
					self.state = "waiting_for_address_prompt"

				else:
					print(f"Received: {line}")

			except Exception as e:
				print(f"Writer error: {e}")
				break

	def run(self):
		"""Main function to start communication"""
		try:
			# Open serial connection
			self.ser = serial.Serial(
				port=self.tty_device,
				baudrate=115200,
				timeout=1,
				parity=serial.PARITY_NONE,
				stopbits=serial.STOPBITS_ONE,
				bytesize=serial.EIGHTBITS
			)

			print(f"Connected to {self.tty_device}")

			# Start threads
			reader = threading.Thread(target=self.reader_thread, daemon=True)
			writer = threading.Thread(target=self.writer_thread, daemon=True)

			reader.start()
			writer.start()

			# Wait for completion or user interrupt
			try:
				writer.join()
			except KeyboardInterrupt:
				print("\nInterrupted by user")

		except Exception as e:
			print(f"Error: {e}")
		finally:
			self.running = False
			if self.ser:
				self.ser.close()

def main():
	if len(sys.argv) != 3:
		usage()

	tty_device = sys.argv[1]
	kernel_image = sys.argv[2]

	# Check if TTY device exists
	if not os.path.exists(tty_device):
		print(f"Error: TTY device '{tty_device}' does not exist")
		sys.exit(1)

	# Check if kernel image exists
	if not os.path.isfile(kernel_image):
		print(f"Error: Kernel image '{kernel_image}' does not exist")
		sys.exit(1)

	# Create and run kernel sender
	sender = KernelSender(tty_device, kernel_image)
	sender.run()

if __name__ == "__main__":
	main()