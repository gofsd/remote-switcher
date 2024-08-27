items = [
    {
        name: "setting",
        value: "settings",
        type: "menu",
        parent: ""
    },
    {   name: "ssid",
        type: "string",
        value: "esp8266",
        parent: "setting"

    },
    {
        name: "password",
        type: "string",
        value: "Password1",
        parent: "setting"

    },
    {
        name: "isSTA",
        type: "bool",
        value: "false",
        parent: "setting"

    },
    {
        name: "secret",
        type: "string",
        value: "very secret string",
        parent: "setting"
    },
    {
        name: "cer",
        type: "ssl_string",
        value: `-----BEGIN CERTIFICATE-----
MIIESDCCAzCgAwIBAgIQFcG+aOOfm9y4vxrnrbETVTANBgkqhkiG9w0BAQsFADCB
vTELMAkGA1UEBhMCSU4xFDASBgNVBAgTC01haHJhc2hhdHJhMQ8wDQYDVQQHEwZN
dW1iYWkxMjAwBgNVBAoTKUNsb3VkIE1hbnRoYW4gU29mdHdhcmUgU29sdXRpb25z
IFB2dCBMdGQuMTMwMQYDVQQLEypDbG91ZCBNYW50aGFuIFJvb3QgQ2VydGlmaWNh
dGlvbiBBdWh0b3JpdHkxHjAcBgNVBAMTFUNsb3VkIE1hbnRoYW4gUm9vdCBDQTAe
Fw0yNDA3MDgyMDE0NDJaFw0yOTA3MDcyMDE0NDJaMIG9MQswCQYDVQQGEwJJTjEU
MBIGA1UECBMLTWFocmFzaGF0cmExDzANBgNVBAcTBk11bWJhaTEyMDAGA1UEChMp
Q2xvdWQgTWFudGhhbiBTb2Z0d2FyZSBTb2x1dGlvbnMgUHZ0IEx0ZC4xMzAxBgNV
BAsTKkNsb3VkIE1hbnRoYW4gUm9vdCBDZXJ0aWZpY2F0aW9uIEF1aHRvcml0eTEe
MBwGA1UEAxMVQ2xvdWQgTWFudGhhbiBSb290IENBMIIBIjANBgkqhkiG9w0BAQEF
AAOCAQ8AMIIBCgKCAQEArUYbsF6s8QQyuLVQDuJfZFmOytmIOcKw/58yZY6uJ/pq
4pn26RthjbUFBVUUmMNfIUfysXZUTb55hEB+/CzOFEcSqTp/8qweCdgdu/Xzi3BK
0wXOJtex8bqf1JbmXeQqn7pYpW2iy4769nbGLnS3a7nxuOgyjOWsxeAunwVM8lIJ
bQsLCphhSFVLaq8f5MT+KEDoB+6ymkjhO9Uzq5ExO7shfM89M+nS2ANXVpdEV3v7
mIb/cktKE5x0kvt/f93+1LEGG5816I2sgbeYnc5wOfvgrqlRpPbdWoWoUuoHXJ9c
S8oR7QvpDn0Wh9BXsV6yC2YWBiHdcrsskP/6lmerLQIDAQABo0IwQDAOBgNVHQ8B
Af8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUjUF8e5h3Ap+CAHDJ
V9oiIFUz3kowDQYJKoZIhvcNAQELBQADggEBAA6kYEW0jwgQBWFO1Kaas3q9GImq
NYp8sieMvFQVjBedJ/++Y/YjP8VE3I6s5seR72D96bb7Hd+H+XtMHUvts84WVTzI
iI/1IsDwKcK5jKW/XajTkU9fB0V2+mOd4H81l3d7gqfySr34Fx6y+CEHcfPA1Hb3
cb1PI9hhS7DEWeym+k3KJwRYeUwecELNfa4RhcwhYOcVZGWbejarCQTNmhFiwPHT
ZKjr6nSxD3VIPjKzvzu3RwmXyxcrDy0DPOTJ9WYj8MXhSbbsLi8COX16+ZeObAig
v98mBEXLNgSIkhAwvV/s1KfxaKyM1VWO0rc+n2gOjH/KS9Ee9tKSdqXuJlQ=
-----END CERTIFICATE-----`,
parent: "setting"

    },
    {
        name: "key",
        type: "privat_key_string",
        value: `-----BEGIN RSA PRIVATE KEY-----
MIIEowIBAAKCAQEArUYbsF6s8QQyuLVQDuJfZFmOytmIOcKw/58yZY6uJ/pq4pn2
6RthjbUFBVUUmMNfIUfysXZUTb55hEB+/CzOFEcSqTp/8qweCdgdu/Xzi3BK0wXO
Jtex8bqf1JbmXeQqn7pYpW2iy4769nbGLnS3a7nxuOgyjOWsxeAunwVM8lIJbQsL
CphhSFVLaq8f5MT+KEDoB+6ymkjhO9Uzq5ExO7shfM89M+nS2ANXVpdEV3v7mIb/
cktKE5x0kvt/f93+1LEGG5816I2sgbeYnc5wOfvgrqlRpPbdWoWoUuoHXJ9cS8oR
7QvpDn0Wh9BXsV6yC2YWBiHdcrsskP/6lmerLQIDAQABAoIBAQCdEm2tPtXFEgos
uGffZkQVAr5XkkRUH/I1dwnHWET1hqL30ZwrCiAvxkXL5KlsJccJs2AEkQCxDRa4
1YKHdNJHPXpkH9y2jmIYiGnD5pwk7eFWU+/h9CuhtoyGuVgRCvG8o78SD0XBA1bq
rxbOLA3NwNynww13/gfVxHhevP5Ytpk/je4LU5Qf49pf/Mun2mNGCPekUM6cGfxH
MMFTszompeanSKNY2P+bJX0PtlhHaKPQF85kWKnOVuxsz8VYJ4L5a6rV9xJY8Yul
kyiHtljCtm5Xc92bcKzAH+l0M4mgdPtiHZRFGgmAFkXTwwBLOxBe+RF/RukfFhJb
pQMPK7zdAoGBAMw19PWXI+3wW8zG73lWKLy6JaBAuIRH2yqeNXMVhdwKbk5MdVjL
09OHiI4T4Q9aPe4Qf2XqPnhdjkrYY5NoXRNuyKjQ5Ze9qlga+slONEpeuysu/egK
PbKt4EV4POLEylKOcrfiPHGc6z2hgeyATjoCZAXhkGHYdVsOgLHQuQXPAoGBANk3
n8dj7S1PWQxPnMzQm8kMh66JTPsUE9UPD63h8mah0KkyHvVFNfb0PiFWhNvs3j0i
oRr9SsmxH1/mXVOGE3V73ThO8a/Ieht1z8hJo4xetcLuQxR+amDKAWaCAf6QsZQ9
BBtNjYwpS0F6MQNan8mPIa5pJEiujyc7bOlXgfpDAoGAIx+Eqlj8jQgyQd5VggEX
sJ4zqoR3OAlD4OilOM4S3+3LWxw7crJcFJ3TFtzEcOeMj023G8hWRk2RE750If5p
Q9ZiHOcRFjBBBDZfvA//Xms8tgi3Exnv0rOlR02O61H9moV1pbr7um7K5ybIOe03
hzKyEXDibHHWfXrZF7xWlnECgYAW+OYB/VRxDUo6nhTUKF/BZHzY/ZZRm275gm1D
E3lCV2ys6CsT/2zUoEIN3ouQgr2CM65cH1uQdxX9W2yVT8GxFBdyRA8VaxtW2h/O
a9NLHh0U74PSoAf6EoMRZ0B0vrK5HbbYeFw27YsS8qxKUYRCmcuTGXWH5kYnfQZo
qyOI5wKBgCsYvXWcfY6XLqGaMh2TectluYrT1kRiklEyjlZu4wgOuBupZDuHgFfB
aTC6nFPpmzoOmzHlsYiUB7/FM+U5HqqtonbCME2PfHJVoDG1v+xGDv9laDIPdkAe
qRs8jPUYUvoiV4IpVgy+xRB5ioMm4K+QnWyDG9dcnA693yf1rUb3
-----END RSA PRIVATE KEY-----`,
parent: "setting"

    },
    {   name: "date",
        type: "datetime-local-string",
        value: "2024-08-10T07:40:01.9",
        parent: "setting"

    },
    {
        name: "pin",
        type: "menu",
        value: "pins",
        parent: ""

    },
    {
        name: "cron",
        type: "menu",
        value:"crons",
        parent: ""

    },
    {
        name: "1",
        type: "analogInput",
        value: 12,
        parent: "pin"
    },
    {
        name: "2",
        type: "digitalInput",
        value: 1,
        parent: "pin"
    },
    {
        name: "3",
        type: "digitalOutput",
        value: 1,
        parent: "pin"
    },
    {
        name: "4",
        type: "pwmOutput",
        value: 1,
        parent: "pin"
    },
]

var defaultMenuItem = "settings"
var active_menu_item = null
function onSubmit(form, methodType, url) {
    var xhr = new XMLHttpRequest();
    var formData = JSON.stringify(Object.fromEntries(new FormData(form)));
    console.log(formData)
    xhr.open(methodType, url)
    xhr.setRequestHeader("Content-Type", "application/json");

    xhr.send(formData);

    xhr.onreadystatechange = function () {
        if (xhr.readyState == XMLHttpRequest.DONE) {
            form.reset();
        }
    }
    return false;
}


function init() {
    createMenu()
    createForms()
}

function createForms() {
    form = document.createElement("form")
    current = localStorage.getItem("active_menu_item")
    item = items.find(i=>i.name === current)
    form.setAttribute("action", item.path)
    form.setAttribute("method", "put")
    form.setAttribute("onsubmit", "handleSubmit(event)")
    // submitElement = document.createElement("input")
    // submitElement.setAttribute("type", "submit")
    // submitElement.setAttribute("value", "submit")
    //form.appendChild(submitElement)
    root.appendChild(form)
}

function handleSubmit(e) {
    e.preventDefault();
    form = e.target
    methodType = form.getAttribute("method")
    url = form.getAttribute("action")
    onSubmit(form, methodType, url)
}

function handleDelete(e) {
    e.preventDefault();
    form = e.target.parentElement
    url = form.getAttribute("action")
    input = document.getElementById(url)
    url = url + `/?${url}=` + input.getAttribute("value")
    console.log(form, url)
    onSubmit(form, "delete", url)

    console.log(e)
}

function createMenu() {
    menu = document.createElement("div")
    menu.setAttribute("class", "topnav")

    for (item of items) {
        a = createMenueItem(item)
        menu.appendChild(a)
    }
    root.appendChild(menu)
}

function createMenueItem(item) {
    a = document.createElement("a")
    a.innerHTML = item.name
    a.setAttribute("onclick", "handleClick(event)")
    a.setAttribute("href", "javascript:void(0)")
    val = localStorage.getItem("active_menu_item")
    if (val != null && a.innerHTML == val) {
        active_menu_item = a
        a.setAttribute("class", "active")
        return a
    } else if (val == null) {
        n = item.name
        if (n == defaultMenuItem) {
            active_menu_item = a
            a.setAttribute("class", "active")
            return a
        }
    }
    return a
}

function handleClick(e) {
    target = getTargetFromEvent(e)
    if ("active" != target.getAttribute("class")) {
        handleMenuItemClick(target)
    }
}

function getTargetFromEvent(e) {
    return e.target
}

function handleMenuItemClick(t) {
    if (active_menu_item != null && t.innerHTML == active_menu_item.innerHTML) {
        return
    } else {
        t.setAttribute("class", "active")
        if (active_menu_item != null) {
            active_menu_item.removeAttribute("class")

        }
        active_menu_item = t
        localStorage.setItem("active_menu_item", t.innerHTML)
    }
}

init()