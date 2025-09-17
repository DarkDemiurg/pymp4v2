FROM ubuntu:24.04

# Установка переменных окружения для избежания интерактивных запросов
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC

# Установка системных зависимостей
RUN apt-get update && apt-get install -y \
    software-properties-common \
    build-essential \
    cmake \
    ninja-build \
    git \
    wget \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Добавление репозитория с несколькими версиями Python
RUN add-apt-repository ppa:deadsnakes/ppa -y && \
    apt-get update && \
    apt-get install -y \
    python3.9 \
    python3.9-dev \
    python3.9-venv \
    python3.10 \
    python3.10-dev \
    python3.10-venv \
    python3.11 \
    python3.11-dev \
    python3.11-venv \
    python3.12 \
    python3.12-dev \
    python3.12-venv \
    python3.13 \
    python3.13-dev \
    python3.13-venv \
    && rm -rf /var/lib/apt/lists/*

# Создание симлинков для удобства
RUN ln -s /usr/bin/python3.9 /usr/local/bin/python3.9 && \
    ln -s /usr/bin/python3.10 /usr/local/bin/python3.10 && \
    ln -s /usr/bin/python3.11 /usr/local/bin/python3.11 && \
    ln -s /usr/bin/python3.12 /usr/local/bin/python3.12 && \
    ln -s /usr/bin/python3.13 /usr/local/bin/python3.13

# Установка pip для каждой версии Python
RUN for py in 3.9 3.10 3.11 3.12 3.13; do \
    curl -sS https://bootstrap.pypa.io/get-pip.py | python$py; \
    done

# Установка tox
RUN python3.13 -m pip install tox

# Создание рабочей директории
WORKDIR /app

# Копирование файлов проекта
COPY . .

# Команда для запуска тестов
CMD ["tox"]
