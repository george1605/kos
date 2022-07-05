from tkinter import *
from tkinter import ttk
__interface__ = 0

class window:
  width = 300
  height = 300
  x = 20
  y = 20
  name = "Window"
  def __init__(self,Width,Height):
     width = Width
     height = Height
  def show(self):
     frm = ttk.Frame(Tk(),width=self.width,height=self.height)

def kmain():
  print("Hello Kernel!")
  
def kmain2():
  myWin = window(200,200)
  
  
if(__interface__ == 0)
 kmain()
else:
 kmain2()
