const API_BASE = '/api/modbus';
let autoRefresh = false;
let refreshInterval = null;
let refreshRate = 5000;

async function apiCall(endpoint, method = 'GET', data = null) {
    try {
        const options = {
            method: method,
            headers: {
                'Content-Type': 'application/json',
            }
        };

        if (data) {
            options.body = JSON.stringify(data);
        }

        const response = await fetch(`${API_BASE}${endpoint}`, options);
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        return await response.json();
    } catch (error) {
        console.error('API call failed:', error);
        throw error;
    }
}

async function loadLoggingConfig() {
    try {
        const response = await fetch('/api/modbus/logging-config');
        const data = await response.json();
        const checkbox = document.getElementById('modbus-logging');
        if (checkbox) {
            checkbox.checked = data.logging_enabled;
        }
        console.log('Logging config loaded:', data.logging_enabled ? 'enabled' : 'disabled');
    } catch (error) {
        console.error('Failed to load logging config:', error);
    }
}

async function toggleModbusLogging(enabled) {
    try {
        const response = await fetch('/api/modbus/logging-config', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ logging_enabled: enabled })
        });
        const data = await response.json();
        if (data.status === 'ok') {
            console.log('Logging config saved:', enabled ? 'enabled' : 'disabled');
        }
    } catch (error) {
        console.error('Failed to save logging config:', error);
    }
}

function getStatusClass(status) {
    switch (status) {
        case 0: return 'status-unknown';
        case 1: return 'status-online';
        case 2: return 'status-offline';
        case 3: return 'status-error';
        default: return 'status-unknown';
    }
}

function getStatusText(status) {
    switch (status) {
        case 0: return 'Unknown';
        case 1: return 'Online';
        case 2: return 'Offline';
        case 3: return 'Error';
        default: return 'Unknown';
    }
}

function formatTimestamp(ms) {
    if (!ms) return 'Never';
    const date = new Date(ms);
    return date.toLocaleTimeString();
}

function scaledValue(raw, scale, offset) {
    return (raw * scale + offset).toFixed(2);
}

function getRegisterTypeName(type) {
    switch (type) {
        case 1: return 'Coil';
        case 2: return 'Discrete';
        case 3: return 'Holding';
        case 4: return 'Input';
        default: return 'Unknown';
    }
}

function showTab(tabName) {
    document.querySelectorAll('.tab').forEach(tab => tab.classList.remove('active'));
    document.querySelectorAll('.tab-content').forEach(content => content.classList.remove('active'));
    
    event.target.classList.add('active');
    document.getElementById(`${tabName}-tab`).classList.add('active');
}

function openModal(deviceId) {
    document.getElementById('add-register-modal').style.display = 'block';
    document.getElementById('register-device-id').value = deviceId;
}

function closeModal() {
    document.getElementById('add-register-modal').style.display = 'none';
    document.getElementById('add-register-form').reset();
}

async function loadDevices() {
    try {
        const devices = await apiCall('/devices');
        renderDevices(devices);
    } catch (error) {
        console.error('Failed to load devices:', error);
        alert('Failed to load devices. Please refresh the page.');
    }
}

function renderDevices(devices) {
    const container = document.getElementById('devices-list');
    if (!container) return;

    if (devices.length === 0) {
        container.innerHTML = '<div class="card"><p>No devices configured. Add a device above or use presets.</p></div>';
        return;
    }

    container.innerHTML = devices.map(device => `
        <div class="device-card">
            <div class="device-header">
                <div>
                    <div class="device-title">${device.name}</div>
                    <div style="color: #6b7280; font-size: 0.9em;">
                        ID: ${device.device_id} | ${device.description || 'No description'}
                    </div>
                </div>
                <div class="status-badge ${getStatusClass(device.status)}">
                    ${getStatusText(device.status)}
                </div>
            </div>
            <div style="margin-bottom: 15px; font-size: 0.9em; color: #6b7280;">
                <span>Baudrate: ${device.baudrate}</span> |
                <span>Poll Interval: ${device.poll_interval_ms}ms</span> |
                <span>Polls: ${device.poll_count}</span> |
                <span>Errors: ${device.error_count}</span>
            </div>
            <div>
                <button class="btn btn-sm btn-secondary" onclick="openModal(${device.device_id})">
                    Add Register
                </button>
                <button class="btn btn-sm btn-secondary" onclick="deleteDevice(${device.device_id})">
                    Delete Device
                </button>
            </div>
            ${device.registers.length > 0 ? `
                <table class="register-table">
                    <thead>
                        <tr>
                            <th>Addr</th>
                            <th>Type</th>
                            <th>Name</th>
                            <th>Value</th>
                            <th>Unit</th>
                            <th>Actions</th>
                        </tr>
                    </thead>
                    <tbody>
                        ${device.registers.map(reg => `
                            <tr>
                                <td>${reg.address}</td>
                                <td>${getRegisterTypeName(reg.type)}</td>
                                <td>${reg.name}</td>
                                <td>${reg.writable ? `
                                    <input type="number" class="write-input" 
                                           id="write-${device.device_id}-${reg.address}"
                                           value="${scaledValue(reg.last_value, reg.scale, reg.offset)}"
                                           ${reg.writable ? '' : 'disabled'}>
                                    <button class="btn btn-sm btn-primary" 
                                            onclick="writeRegister(${device.device_id}, ${reg.address})">
                                        Write
                                    </button>
                                ` : scaledValue(reg.last_value, reg.scale, reg.offset)}
                                </td>
                                <td>${reg.unit || ''}</td>
                                <td>
                                    <button class="btn btn-sm btn-secondary" 
                                            onclick="deleteRegister(${device.device_id}, ${reg.address})">
                                        Delete
                                    </button>
                                </td>
                            </tr>
                        `).join('')}
                    </tbody>
                </table>
            ` : '<p style="color: #6b7280; margin-top: 10px;">No registers configured.</p>'}
        </div>
    `).join('');
}

async function addDevice(event) {
    event.preventDefault();
    const form = event.target;
    const formData = new FormData(form);
    
    const device = {
        device_id: parseInt(formData.get('device_id')),
        name: formData.get('name'),
        description: formData.get('description'),
        poll_interval_ms: parseInt(formData.get('poll_interval')),
        baudrate: parseInt(formData.get('baudrate')),
        enabled: formData.get('enabled') === 'on'
    };

    try {
        await apiCall('/devices', 'POST', device);
        form.reset();
        loadDevices();
        alert('Device added successfully!');
    } catch (error) {
        alert('Failed to add device: ' + error.message);
    }
}

async function deleteDevice(deviceId) {
    if (!confirm('Are you sure you want to delete this device and all its registers?')) {
        return;
    }

    try {
        await apiCall(`/devices?device_id=${deviceId}`, 'DELETE');
        loadDevices();
        alert('Device deleted successfully!');
    } catch (error) {
        alert('Failed to delete device: ' + error.message);
    }
}

async function addRegister(event) {
    event.preventDefault();
    const form = event.target;
    const formData = new FormData(form);
    
    const register = {
        device_id: parseInt(formData.get('device_id')),
        address: parseInt(formData.get('address')),
        type: parseInt(formData.get('type')),
        name: formData.get('name'),
        unit: formData.get('unit'),
        scale: parseFloat(formData.get('scale')),
        offset: parseFloat(formData.get('offset')),
        writable: formData.get('writable') === 'on',
        description: formData.get('description')
    };

    try {
        await apiCall('/registers', 'POST', register);
        closeModal();
        loadDevices();
        alert('Register added successfully!');
    } catch (error) {
        alert('Failed to add register: ' + error.message);
    }
}

async function deleteRegister(deviceId, address) {
    if (!confirm('Are you sure you want to delete this register?')) {
        return;
    }

    try {
        await apiCall(`/registers?device_id=${deviceId}&address=${address}`, 'DELETE');
        loadDevices();
        alert('Register deleted successfully!');
    } catch (error) {
        alert('Failed to delete register: ' + error.message);
    }
}

async function writeRegister(deviceId, address) {
    // Try dashboard ID first, fall back to device list ID
    let input = document.getElementById(`dash-write-${deviceId}-${address}`);
    if (!input) {
        input = document.getElementById(`write-${deviceId}-${address}`);
    }

    if (!input) {
        alert('Input field not found for write operation!');
        return;
    }

    const value = parseFloat(input.value);

    if (isNaN(value)) {
        alert('Please enter a valid number');
        return;
    }

    try {
        await apiCall(`/write?device_id=${deviceId}&address=${address}`, 'POST', { value });
        alert('Register written successfully!');
        loadDevices();
    } catch (error) {
        alert('Failed to write register: ' + error.message);
    }
}

function addPreset(presetType) {
    let device, registers;

    if (presetType === 'eairmd') {
        device = {
            device_id: 1,
            name: 'eAirMD',
            description: 'Enervent eAirMD ventilation unit',
            poll_interval_ms: 5000,
            baudrate: 19200,
            enabled: true
        };
        registers = [
            { address: 1, type: 3, name: 'Room Temp TE20', unit: '°C', scale: 0.1, offset: 0, writable: false, description: 'Room temperature sensor TE20' },
            { address: 6, type: 3, name: 'Fresh Air', unit: '°C', scale: 0.1, offset: 0, writable: false, description: 'Fresh air temperature TE01' },
            { address: 8, type: 3, name: 'Supply Air', unit: '°C', scale: 0.1, offset: 0, writable: false, description: 'Supply air temperature TE10' },
            { address: 13, type: 3, name: 'Exhaust Humidity', unit: '%RH', scale: 1, offset: 0, writable: false, description: 'Exhaust air relative humidity' },
            { address: 44, type: 3, name: 'Mode Status', unit: '', scale: 1, offset: 0, writable: false, description: 'Current operating mode' },
            { address: 135, type: 3, name: 'Setpoint', unit: '°C', scale: 0.1, offset: 0, writable: true, description: 'Supply air temperature setpoint' }
        ];
    } else if (presetType === 'ewind') {
        device = {
            device_id: 1,
            name: 'eWind',
            description: 'Enervent eWind ventilation unit',
            poll_interval_ms: 5000,
            baudrate: 9600,
            enabled: true
        };
        registers = [
            { address: 6, type: 3, name: 'Fresh Air', unit: '°C', scale: 0.1, offset: 0, writable: false, description: 'Fresh air temperature' },
            { address: 8, type: 3, name: 'Supply Air', unit: '°C', scale: 0.1, offset: 0, writable: false, description: 'Supply air temperature' },
            { address: 13, type: 3, name: 'Exhaust Humidity', unit: '%RH', scale: 1, offset: 0, writable: false, description: 'Exhaust air relative humidity' },
            { address: 48, type: 3, name: 'Display Setpoint', unit: '°C', scale: 0.1, offset: 0, writable: false, description: 'Temperature controller setpoint' },
            { address: 135, type: 3, name: 'Setpoint', unit: '°C', scale: 0.1, offset: 0, writable: true, description: 'Supply air temperature setpoint' }
        ];
    }

    addPresetDevice(device, registers);
}

async function addPresetDevice(device, registers) {
    try {
        await apiCall('/devices', 'POST', device);
        
        for (const reg of registers) {
            await apiCall('/registers', 'POST', { ...reg, device_id: device.device_id });
        }
        
        loadDevices();
        alert(`${device.name} preset added successfully with ${registers.length} registers!`);
    } catch (error) {
        alert('Failed to add preset: ' + error.message);
    }
}

async function refreshDashboard() {
    const container = document.getElementById('dashboard-content');
    if (!container) return;

    try {
        const devices = await apiCall('/devices');
        
        if (devices.length === 0) {
            container.innerHTML = '<div class="card"><p>No devices configured. <a href="/modbus.html">Configure devices</a></p></div>';
            document.getElementById('dashboard-status').className = 'status-indicator status-unknown';
            return;
        }

        let anyOnline = false;
        container.innerHTML = devices.map(device => `
            <h3 style="margin: 20px 0 15px 0; display: flex; align-items: center;">
                <span class="status-indicator ${getStatusClass(device.status)}"></span>
                ${device.name} (ID: ${device.device_id})
            </h3>
            <div class="dashboard-grid">
                ${device.registers.map(reg => `
                    <div class="value-card">
                        <div class="value-label">${reg.name}</div>
                        <div class="value-display">
                            ${scaledValue(reg.last_value, reg.scale, reg.offset)}
                            <span class="value-unit">${reg.unit || ''}</span>
                        </div>
                        ${reg.writable ? `
                            <div style="margin-top: 10px;">
                                <input type="number" class="write-input" 
                                       id="dash-write-${device.device_id}-${reg.address}"
                                       value="${scaledValue(reg.last_value, reg.scale, reg.offset)}"
                                       placeholder="New value">
                                <button class="btn btn-sm btn-primary" 
                                        onclick="writeRegister(${device.device_id}, ${reg.address})">
                                    Write
                                </button>
                            </div>
                        ` : ''}
                        <div class="value-timestamp">
                            Updated: ${formatTimestamp(reg.last_update)}
                        </div>
                    </div>
                `).join('')}
            </div>
        `).join('');

        const overallStatus = devices.some(d => d.status === 1) ? 'status-online' :
                              devices.some(d => d.status === 3) ? 'status-error' : 'status-offline';
        document.getElementById('dashboard-status').className = `status-indicator ${overallStatus}`;
    } catch (error) {
        console.error('Failed to refresh dashboard:', error);
        container.innerHTML = '<div class="card"><p style="color: #ef4444;">Failed to load data. Please refresh.</p></div>';
    }
}

function refreshData() {
    const btn = event ? event.target : document.querySelector('.refresh-btn');
    if (btn) {
        btn.classList.add('loading');
    }
    
    if (document.getElementById('dashboard-content')) {
        refreshDashboard();
    } else if (document.getElementById('devices-list')) {
        loadDevices();
    }
    
    if (btn) {
        setTimeout(() => btn.classList.remove('loading'), 500);
    }
}

function toggleAutoRefresh() {
    autoRefresh = !autoRefresh;
    const btn = document.getElementById('auto-refresh-btn');
    
    if (autoRefresh) {
        btn.textContent = `Auto Refresh: ON (${refreshRate/1000}s)`;
        btn.classList.remove('btn-secondary');
        btn.classList.add('btn-primary');
        refreshInterval = setInterval(refreshData, refreshRate);
    } else {
        btn.textContent = 'Auto Refresh: OFF';
        btn.classList.remove('btn-primary');
        btn.classList.add('btn-secondary');
        clearInterval(refreshInterval);
    }
}

document.addEventListener('DOMContentLoaded', function() {
    const addDeviceForm = document.getElementById('add-device-form');
    if (addDeviceForm) {
        addDeviceForm.addEventListener('submit', addDevice);
    }

    const addRegisterForm = document.getElementById('add-register-form');
    if (addRegisterForm) {
        addRegisterForm.addEventListener('submit', addRegister);
    }

    if (document.getElementById('devices-list')) {
        loadDevices();
    }

    if (document.getElementById('dashboard-content')) {
        loadLoggingConfig();
        refreshDashboard();
    }
});