# Testing Guide - Assignment 3

## 🚀 Quick Start

### Compile the Project:
```bash
cd build
cmake --build .
```

### Run the Simulation:
```bash
# From project root
.\build\assignment3.exe r 0.005
```

---

## 🧪 Test Scenarios

### Test 1: RK4 Integrator (Recommended)
```bash
.\build\assignment3.exe r 0.005
```
**Expected:**
- ✅ All three simulations running smoothly
- ✅ Red sphere: nearly perfect circle
- ✅ Pendulum: stable swinging motion
- ✅ Cloth: realistic draping, no explosions

---

### Test 2: Trapezoidal Integrator
```bash
.\build\assignment3.exe t 0.001
```
**Expected:**
- ✅ Simulations run (may be slightly less stable than RK4)
- ⚠️ Red sphere: slight outward drift
- ✅ Pendulum: stable but requires smaller timestep
- ⚠️ Cloth: may be unstable with larger timesteps

---

### Test 3: Forward Euler (Unstable)
```bash
.\build\assignment3.exe e 0.0001
```
**Expected:**
- ⚠️ Red sphere: visible outward spiral
- ⚠️ Pendulum: may require very small timesteps
- ❌ Cloth: will likely explode (expected behavior!)

---

### Test 4: Stability Comparison
```bash
# Large timestep - RK4 handles it well
.\build\assignment3.exe r 0.01

# Same timestep - Euler explodes
.\build\assignment3.exe e 0.01
```
**Purpose:** Demonstrates why RK4 is superior for stiff systems

---

### Test 5: Reset Functionality
1. Run any simulation
2. Wait for motion to develop
3. Press **'R'** key
4. Verify all systems reset to initial state

---

## 🎯 What to Look For

### Scene Layout:
```
Left: Red sphere (circular motion)
Middle: Blue spheres (pendulum chain)
Right: Wireframe grid (cloth)
```

### Circular Motion (Left):
- [ ] Red sphere visible
- [ ] Moving in circular path
- [ ] Euler spirals outward
- [ ] RK4 maintains radius

### Pendulum (Middle):
- [ ] 4 blue spheres in chain
- [ ] White lines connecting them
- [ ] Top sphere stays fixed
- [ ] Natural swinging motion
- [ ] Eventually settles

### Cloth (Right):
- [ ] 8×8 grid of lines visible
- [ ] Hanging from two top corners
- [ ] Falls and drapes naturally
- [ ] No explosions (with RK4)
- [ ] Stable equilibrium shape

---

## 📊 Performance Benchmarks

| Integrator | Timestep | FPS (approx) | Stability |
|------------|----------|--------------|-----------|
| RK4 | 0.005s | 60+ | ⭐⭐⭐⭐⭐ |
| RK4 | 0.01s | 60+ | ⭐⭐⭐⭐ |
| Trapezoidal | 0.001s | 60+ | ⭐⭐⭐⭐ |
| Trapezoidal | 0.005s | 60+ | ⭐⭐ |
| Euler | 0.0001s | 60+ | ⭐⭐ |
| Euler | 0.001s | 60+ | ❌ |

---

## 🐛 Troubleshooting

### Problem: Simulation explodes immediately
**Solution:** 
- Use RK4 integrator
- Reduce timestep
- Check that you're using `r 0.005` not `e 0.005`

### Problem: Cloth doesn't move
**Solution:**
- Check gravity is enabled
- Verify particles aren't all fixed
- Increase timestep to see motion faster

### Problem: Nothing appears on screen
**Solution:**
- Check camera distance (should be ~10 units back)
- Verify lighting is working
- Try zooming out with mouse scroll

### Problem: Cloth tears apart
**Solution:**
- This is expected with Euler!
- Use RK4 integrator
- Reduce spring stiffness
- Increase drag coefficient

---

## 🎮 Interactive Controls

| Key/Action | Effect |
|------------|--------|
| R | Reset all simulations |
| Left Mouse Drag | Rotate camera |
| Right Mouse Drag | Pan camera |
| Mouse Scroll | Zoom in/out |
| ESC | Exit application |

---

## 📈 Physics Tuning

### To make cloth more stable:
```cpp
// In SimulationApp.cpp, cloth section:
system->SetDragCoefficient(3.0f);  // Increase from 2.0
```

### To make cloth stiffer:
```cpp
const float structural_stiffness = 120.0f;  // Increase from 80
```

### To make cloth more flexible:
```cpp
const float flex_stiffness = 20.0f;  // Decrease from 40
```

### To change grid resolution:
```cpp
const int grid_size = 10;  // Increase from 8 (10×10 grid)
// Warning: More particles = slower simulation!
```

---

## ✅ Verification Checklist

Before submitting, verify:

- [ ] Project compiles without errors
- [ ] All three simulations visible simultaneously
- [ ] RK4 integrator runs stably with 0.005 timestep
- [ ] Euler shows instability (spiraling/explosions)
- [ ] Trapezoidal is more stable than Euler
- [ ] Reset key 'R' works for all simulations
- [ ] Camera controls work properly
- [ ] No console errors or warnings
- [ ] Cloth has 64 particles (8×8)
- [ ] Cloth has structural, shear, and flex springs
- [ ] Pendulum has 4 particles
- [ ] Code is clean and well-commented

---

## 🎓 Grading Verification

### Simple Example (20%):
```bash
# Test Euler vs Trapezoidal stability
.\build\assignment3.exe e 0.01
.\build\assignment3.exe t 0.01
```
- [ ] Can see outward spiral with Euler
- [ ] Trapezoidal is more stable

### Pendulum (40%):
```bash
.\build\assignment3.exe r 0.005
```
- [ ] 4-particle chain swings
- [ ] Springs visible
- [ ] Gravity working
- [ ] Drag causes settling
- [ ] RK4 implemented

### Cloth (40%):
```bash
.\build\assignment3.exe r 0.005
```
- [ ] 8×8 grid visible
- [ ] Hangs from corners
- [ ] Natural draping motion
- [ ] All three spring types working
- [ ] Wireframe rendering

---

## 🎉 Success Criteria

✅ **PASS** if:
- Compiles without errors
- All three examples run simultaneously
- RK4 is stable with cloth
- Euler shows instability
- Reset functionality works

❌ **ISSUES** if:
- Compilation errors
- Cloth explodes with RK4
- Simulations don't reset
- Fewer than 64 cloth particles
- Missing spring types

---

## 📞 Quick Reference Commands

```bash
# Best overall experience
.\build\assignment3.exe r 0.005

# Show Euler instability
.\build\assignment3.exe e 0.001

# Trapezoidal comparison
.\build\assignment3.exe t 0.001

# Rebuild from scratch
cd build
cmake --build . --clean-first

# Rebuild and run
cd build && cmake --build . && cd .. && .\build\assignment3.exe r 0.005
```

---

**Happy Testing!** 🚀

