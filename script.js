// Knob interaction
const knob = document.getElementById('knob');
const knobProgress = document.getElementById('knobProgress');
const knobIndicator = document.getElementById('knobIndicator');
const gainValue = document.getElementById('gainValue');

let isDragging = false;
let currentGain = 50; // Start at 50%

// Update knob visual appearance
function updateKnob(value) {
    currentGain = Math.max(0, Math.min(100, value));
    
    // Update progress circle (stroke-dashoffset)
    const circumference = 377;
    const offset = circumference - (currentGain / 100) * circumference;
    knobProgress.style.strokeDashoffset = offset;
    
    // Update indicator rotation (-135 to +135 degrees = 270 degree range)
    const rotation = -135 + (currentGain / 100) * 270;
    knobIndicator.style.transform = `rotate(${rotation}deg)`;
    
    // Update value display
    gainValue.textContent = `${Math.round(currentGain)}%`;
    
    // Update color based on value
    const hue = currentGain * 1.2; // 0 to 120 (red to green)
    knobProgress.style.stroke = `hsl(${hue}, 70%, 50%)`;
    gainValue.style.color = `hsl(${hue}, 70%, 60%)`;
}

// Handle mouse/touch events
function startDrag(e) {
    isDragging = true;
    updateFromMouse(e);
}

function stopDrag() {
    isDragging = false;
}

function updateFromMouse(e) {
    if (!isDragging && e.type === 'mousemove') return;
    
    const rect = knob.getBoundingClientRect();
    const centerX = rect.left + rect.width / 2;
    const centerY = rect.top + rect.height / 2;
    
    const clientX = e.clientX || (e.touches && e.touches[0].clientX);
    const clientY = e.clientY || (e.touches && e.touches[0].clientY);
    
    const deltaY = centerY - clientY;
    const deltaX = clientX - centerX;
    
    // Calculate angle
    let angle = Math.atan2(deltaX, deltaY) * (180 / Math.PI);
    
    // Convert angle to 0-100 range
    // Knob range: -135° to +135° (270° total)
    let normalizedAngle = angle + 135;
    if (normalizedAngle < 0) normalizedAngle += 360;
    if (normalizedAngle > 270) {
        // Clamp to nearest edge
        normalizedAngle = normalizedAngle > 315 ? 0 : 270;
    }
    
    const newValue = (normalizedAngle / 270) * 100;
    updateKnob(newValue);
    
    // Send value to C++ plugin
    if (window.juce) {
        window.juce.setGain(currentGain / 100);
    }
}

// Event listeners
knob.addEventListener('mousedown', startDrag);
knob.addEventListener('touchstart', startDrag);

document.addEventListener('mousemove', updateFromMouse);
document.addEventListener('touchmove', updateFromMouse);

document.addEventListener('mouseup', stopDrag);
document.addEventListener('touchend', stopDrag);

// Function called from C++ to update UI
window.updateGainFromPlugin = function(value) {
    updateKnob(value);
};

// Initialize
updateKnob(50);