function showNotification(message, type = 'success') {
    const container = document.getElementById('notification-container');
    if (!container) {
        const notificationContainer = document.createElement('div');
        notificationContainer.id = 'notification-container';
        document.body.appendChild(notificationContainer);
    }
    
    const notification = document.createElement('div');
    notification.className = `notification notification-${type}`;
    notification.innerHTML = message;
    
    const containerEl = document.getElementById('notification-container');
    containerEl.appendChild(notification);
    
    setTimeout(() => {
        notification.classList.add('notification-show');
    }, 10);
    
    setTimeout(() => {
        notification.classList.remove('notification-show');
        setTimeout(() => notification.remove(), 300);
    }, 3000);
}

function updateStatus() {
    fetch('/status')
        .then(response => response.json())
        .then(data => {
            const statusText = document.getElementById('status-text');
            const statusDot = document.getElementById('status-dot');
            const mode = document.getElementById('mode');
            const ip = document.getElementById('ip');
            const currentSsid = document.getElementById('current-ssid');
            const rssi = document.getElementById('rssi');
            
            if (data.connected) {
                statusText.textContent = 'Connected';
                statusDot.classList.add('connected');
                statusDot.classList.remove('disconnected');
            } else {
                statusText.textContent = 'Disconnected';
                statusDot.classList.add('disconnected');
                statusDot.classList.remove('connected');
            }
            
            mode.textContent = data.mode === 'AP' ? 'Access Point' : 'Station';
            ip.textContent = data.ip;
            currentSsid.textContent = data.ssid || '--';
            rssi.textContent = data.rssi ? `${data.rssi} dBm` : '--';
        })
        .catch(error => {
            console.error('Error fetching status:', error);
        });
}

document.getElementById('wifi-form').addEventListener('submit', function(e) {
    e.preventDefault();
    
    const ssid = document.getElementById('ssid').value;
    const password = document.getElementById('password').value;

    if (!ssid || ssid.length === 0) {
        showNotification('Please enter SSID', 'error');
        return;
    }

    const formData = new URLSearchParams();
    formData.append('ssid', ssid);
    formData.append('password', password);

    fetch('/save', {
        method: 'POST',
        body: formData
    })
    .then(response => {
        if (response.ok) {
showNotification('Credentials saved! Device will reboot and connect to WiFi.', 'success');
        } else {
            showNotification('Failed to save credentials', 'error');
        }
    })
    .catch(error => {
        console.error('Error saving credentials:', error);
        showNotification('Error saving credentials', 'error');
    });
});

document.getElementById('clear-btn').addEventListener('click', function() {
    if (confirm('Are you sure you want to clear credentials? The device will reboot in AP mode.')) {
        fetch('/clear', {
            method: 'POST'
        })
        .then(response => {
            if (response.ok) {
                showNotification('Credentials cleared! Device will reboot.');
            } else {
                showNotification('Failed to clear credentials', 'error');
            }
        })
        .catch(error => {
            console.error('Error clearing credentials:', error);
showNotification('Error clearing credentials', 'error');
        });
    }
});

updateStatus();

setInterval(updateStatus, 3000);
