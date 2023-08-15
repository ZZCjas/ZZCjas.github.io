from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver import Keys, ActionChains
import time
import sys

def edit(val,t):
    cmd_ctrl = Keys.COMMAND if sys.platform == 'darwin' else Keys.CONTROL
    driver=webdriver.Edge();
    while True:
        try:
            driver.get('https://note.ms/'+'1')
        except:
            continue
        target=driver.find_element(By.CLASS_NAME,'content')
        if target == val:
            time.sleep(2)
        ActionChains(driver).click(target)\
            .key_down(cmd_ctrl)\
            .send_keys("a")\
            .key_up(cmd_ctrl)\
            .send_keys(val)\
            .perform()
        time.sleep(1)
print("Note.ms自动化修改脚本,By ZZCjas@Github.com")

k=input("请输入要修改成的内容:")

t=input("目标剪贴板的编号:")

edit(k,t)    
