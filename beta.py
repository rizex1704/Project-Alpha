import random
import string

def generate_password(length, use_special_chars, use_digits, use_uppercase, use_lowercase):
    # 가능한 문자 집합 초기화
    char_pool = ""
    
    if use_special_chars:
        char_pool += string.punctuation  # 특수문자 추가
    if use_digits:
        char_pool += string.digits  # 숫자 추가
    if use_uppercase:
        char_pool += string.ascii_uppercase  # 대문자 추가
    if use_lowercase:
        char_pool += string.ascii_lowercase  # 소문자 추가

    # 비밀번호 길이가 0 또는 비어있는 문자 집합이면 생성 불가
    if length <= 0 or not char_pool:
        return "Error: Invalid input parameters."

    # 랜덤 비밀번호 생성
    password = ''.join(random.choice(char_pool) for i in range(length))
    return password

# 예시 호출
password = generate_password(length=12, use_special_chars=True, use_digits=True, use_uppercase=True, use_lowercase=True)
print("Generated Password:", password)