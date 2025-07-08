// DOM Elements
let mobileMenuBtn, navLinks, eventsList, heroSearch, sportFilter, dateFilter, toastWrapper;

// State
let currentEvents = [];
let filteredEvents = [];

// Initialize DOM elements after page loads
function initializeDOMElements() {
    mobileMenuBtn = document.getElementById('mobile-menu-btn');
    navLinks = document.getElementById('nav-links');
    eventsList = document.getElementById('events-list');
    heroSearch = document.getElementById('hero-search');
    sportFilter = document.getElementById('sport-filter');
    dateFilter = document.getElementById('date-filter');
    toastWrapper = document.getElementById('toast-wrapper');

    // Log missing elements for debugging
    if (!eventsList) console.error('Events list element not found');
    if (!heroSearch) console.error('Hero search element not found');
    if (!sportFilter) console.error('Sport filter element not found');
    if (!dateFilter) console.error('Date filter element not found');
    if (!toastWrapper) console.error('Toast wrapper element not found');
}

async function fetchEventsFromAPI() {
    try {
        eventsList.innerHTML = `
            <div class="loading-placeholder">
                <p>Loading events... <span class="spinner"></span></p>
            </div>
        `;
        const res = await fetch('/api/events');
        const data = await res.json();

        if (!res.ok) {
            throw new Error(data.message || `HTTP error! status: ${res.status}`);
        }

        // Validate data format
        if (!Array.isArray(data)) {
            throw new Error('Invalid data format received from API');
        }

        // Validate each event has required properties
        const validEvents = data.filter(event => 
            event.id && 
            event.title && 
            event.sport && 
            event.venue && 
            event.date && 
            event.price !== undefined && 
            event.image
        );

        if (validEvents.length === 0) {
            throw new Error('No valid events found in API response');
        }

        currentEvents = validEvents;
        filteredEvents = [...currentEvents];
        renderEvents();
        showToast('Events loaded successfully!', 'success');
    } catch (err) {
        console.error('Failed to fetch live events:', err);
        const errorMessage = err.message.includes('API key') 
            ? 'API configuration error. Contact support.'
            : 'Could not load events. Please try again or contact support.';
        showToast(errorMessage, 'error', fetchEventsFromAPI);
        eventsList.innerHTML = `
            <div class="no-events" style="text-align: center; padding: 3rem; grid-column: 1 / -1;">
                <i class="fas fa-exclamation-circle" style="font-size: 3rem; color: var(--neutral-light); margin-bottom: 1rem;"></i>
                <h3>Unable to Load Events</h3>
                <p>${errorMessage}</p>
                <button class="btn-primary" onclick="fetchEventsFromAPI()">Retry</button>
            </div>
        `;
    }
}

// Initialize the application
document.addEventListener('DOMContentLoaded', function() {
    // Initialize DOM elements first
    initializeDOMElements();
    
    // Initialize all components
    initializeNavigation();
    initializeSearch();
    initializeModals();
    initializeForms();
    initializeCategoryCards();
    initializeScrollAnimations();
    
    // Load events
    fetchEventsFromAPI();
});

// Navigation functionality
function initializeNavigation() {
    // Mobile menu toggle
    if (mobileMenuBtn && navLinks) {
        mobileMenuBtn.addEventListener('click', function() {
            navLinks.classList.toggle('active');
            const icon = mobileMenuBtn.querySelector('i');
            if (navLinks.classList.contains('active')) {
                icon.classList.replace('fa-bars', 'fa-times');
            } else {
                icon.classList.replace('fa-times', 'fa-bars');
            }
        });
    }

    // Smooth scrolling for navigation links
    document.querySelectorAll('.nav-item').forEach(link => {
        link.addEventListener('click', function(e) {
            e.preventDefault();
            const targetId = this.getAttribute('href');
            const targetSection = document.querySelector(targetId);
            
            if (targetSection) {
                targetSection.scrollIntoView({
                    behavior: 'smooth',
                    block: 'start'
                });
                
                // Close mobile menu
                if (navLinks) {
                    navLinks.classList.remove('active');
                }
                if (mobileMenuBtn) {
                    const icon = mobileMenuBtn.querySelector('i');
                    if (icon) {
                        icon.classList.replace('fa-times', 'fa-bars');
                    }
                }
                
                // Update active link
                document.querySelectorAll('.nav-item').forEach(l => l.classList.remove('active'));
                this.classList.add('active');
            }
        });
    });

    // Navbar scroll effect
    window.addEventListener('scroll', function() {
        const navbar = document.querySelector('.main-nav');
        if (navbar) {
            if (window.scrollY > 100) {
                navbar.style.background = 'rgba(255, 255, 255, 0.98)';
                navbar.style.boxShadow = '0 2px 20px rgba(0, 0, 0, 0.1)';
            } else {
                navbar.style.background = 'rgba(255, 255, 255, 0.95)';
                navbar.style.boxShadow = 'none';
            }
        }
    });
}

// Search functionality
function initializeSearch() {
    if (heroSearch) {
        heroSearch.addEventListener('input', debounce(handleSearch, 300));
    }
    
    if (sportFilter) {
        sportFilter.addEventListener('change', handleSearch);
    }
    
    if (dateFilter) {
        dateFilter.addEventListener('change', handleSearch);
    }

    // Search button
    const searchBtn = document.querySelector('.search-btn');
    if (searchBtn) {
        searchBtn.addEventListener('click', function(e) {
            e.preventDefault();
            handleSearch();
            showToast('Searching for events...', 'success');
        });
    }
}

function handleSearch() {
    const searchTerm = heroSearch?.value.toLowerCase() || '';
    const selectedSport = sportFilter?.value || '';
    const selectedDate = dateFilter?.value || '';

    filteredEvents = currentEvents.filter(event => {
        const matchesSearch = event.title.toLowerCase().includes(searchTerm) ||
                            event.venue.toLowerCase().includes(searchTerm);
        const matchesSport = !selectedSport || event.sport === selectedSport;
        const matchesDate = !selectedDate || checkDateFilter(event.date, selectedDate);
        
        return matchesSearch && matchesSport && matchesDate;
    });

    renderEvents();
}

function checkDateFilter(eventDate, filter) {
    const today = new Date();
    const eventDateObj = new Date(eventDate);
    
    switch (filter) {
        case 'today':
            return eventDateObj.toDateString() === today.toDateString();
        case 'tomorrow':
            const tomorrow = new Date(today);
            tomorrow.setDate(tomorrow.getDate() + 1);
            return eventDateObj.toDateString() === tomorrow.toDateString();
        case 'week':
            const weekFromNow = new Date(today);
            weekFromNow.setDate(weekFromNow.getDate() + 7);
            return eventDateObj >= today && eventDateObj <= weekFromNow;
        case 'month':
            const monthFromNow = new Date(today);
            monthFromNow.setMonth(monthFromNow.getMonth() + 1);
            return eventDateObj >= today && eventDateObj <= monthFromNow;
        default:
            return true;
    }
}

// Event rendering
function renderEvents() {
    if (!eventsList) {
        console.error('Events list element not found');
        showToast('Application error: Unable to display events', 'error');
        return;
    }

    eventsList.innerHTML = ''; // Clear previous content

    if (filteredEvents.length === 0) {
        eventsList.innerHTML = `
            <div class="no-events" style="text-align: center; padding: 3rem; grid-column: 1 / -1;">
                <i class="fas fa-search" style="font-size: 3rem; color: var(--neutral-light); margin-bottom: 1rem;"></i>
                <h3>No events found</h3>
                <p>Try adjusting your search criteria</p>
            </div>
        `;
        return;
    }

    eventsList.innerHTML = filteredEvents.map(event => {
        // Validate event properties before rendering
        if (!event.id || !event.title || !event.venue || !event.date || !event.price || !event.image) {
            console.warn('Invalid event data:', event);
            return '';
        }
        return `
            <div class="event-card fade-in-up" onclick="viewEvent(${event.id})">
                <div class="event-image">
                    <img src="${event.image}" alt="${event.title}" loading="lazy" onerror="this.src='https://via.placeholder.com/400x200/4F46E5/FFFFFF?text=Sports+Event'">
                    <div class="event-date">${formatDate(event.date)}</div>
                </div>
                <div class="event-content">
                    <h3 class="event-title">${event.title}</h3>
                    <div class="event-venue">
                        <i class="fas fa-map-marker-alt"></i>
                        <span>${event.venue}</span>
                    </div>
                    <div class="event-footer">
                        <div class="event-price">
                            <span class="from">From</span> $${event.price}
                        </div>
                        <button class="btn-primary" onclick="event.stopPropagation(); bookEvent(${event.id})">
                            Book Now
                        </button>
                    </div>
                </div>
            </div>
        `;
    }).filter(html => html).join(''); // Filter out empty strings
}

// Modal functionality
function initializeModals() {
    // Close modal when clicking outside
    document.querySelectorAll('.modal-overlay').forEach(modal => {
        modal.addEventListener('click', function(e) {
            if (e.target === this) {
                closeModal(this.id);
            }
        });
    });

    // Close modal with Escape key
    document.addEventListener('keydown', function(e) {
        if (e.key === 'Escape') {
            document.querySelectorAll('.modal-overlay.active').forEach(modal => {
                closeModal(modal.id);
            });
        }
    });
}

function openModal(modalId) {
    const modal = document.getElementById(modalId);
    if (modal) {
        modal.classList.add('active');
        document.body.style.overflow = 'hidden';
    }
}

function closeModal(modalId) {
    const modal = document.getElementById(modalId);
    if (modal) {
        modal.classList.remove('active');
        document.body.style.overflow = '';
    }
}

function switchModal(fromModalId, toModalId) {
    closeModal(fromModalId);
    setTimeout(() => openModal(toModalId), 150);
}

// Form handling
function initializeForms() {
    // Login form
    const loginForm = document.getElementById('login-form');
    if (loginForm) {
        loginForm.addEventListener('submit', function(e) {
            e.preventDefault();
            const email = document.getElementById('login-email')?.value;
            const password = document.getElementById('login-password')?.value;
            
            if (email && password) {
                showToast('Login successful! Welcome back.', 'success');
                closeModal('loginModal');
                // Simulate login success
                setTimeout(() => {
                    updateUIForLoggedInUser(email);
                }, 500);
            } else {
                showToast('Please fill in all fields.', 'error');
            }
        });
    }

    // Register form
    const registerForm = document.getElementById('register-form');
    if (registerForm) {
        registerForm.addEventListener('submit', function(e) {
            e.preventDefault();
            const firstName = document.getElementById('register-firstname')?.value;
            const lastName = document.getElementById('register-lastname')?.value;
            const email = document.getElementById('register-email')?.value;
            const password = document.getElementById('register-password')?.value;
            const confirmPassword = document.getElementById('register-confirm')?.value;
            
            if (!firstName || !lastName || !email || !password || !confirmPassword) {
                showToast('Please fill in all fields.', 'error');
                return;
            }
            
            if (password !== confirmPassword) {
                showToast('Passwords do not match.', 'error');
                return;
            }
            
            if (password.length < 6) {
                showToast('Password must be at least 6 characters.', 'error');
                return;
            }
            
            showToast('Account created successfully! Welcome to SportsSeat.', 'success');
            closeModal('registerModal');
            // Simulate registration success
            setTimeout(() => {
                updateUIForLoggedInUser(email);
            }, 500);
        });
    }

    // Newsletter form
    const newsletterForm = document.getElementById('newsletter-form');
    if (newsletterForm) {
        newsletterForm.addEventListener('submit', function(e) {
            e.preventDefault();
            const email = this.querySelector('input[type="email"]')?.value;
            
            if (email) {
                showToast('Thank you for subscribing to our newsletter!', 'success');
                this.reset();
            } else {
                showToast('Please enter a valid email address.', 'error');
            }
        });
    }
}

// Category cards functionality
function initializeCategoryCards() {
    document.querySelectorAll('.category-item').forEach(card => {
        card.addEventListener('click', function() {
            const sport = this.dataset.sport;
            if (sport) {
                // Update sport filter
                if (sportFilter) {
                    sportFilter.value = sport;
                }
                // Clear other filters
                if (heroSearch) heroSearch.value = '';
                if (dateFilter) dateFilter.value = '';
                
                handleSearch();
                
                // Scroll to events section
                const eventsSection = document.getElementById('events');
                if (eventsSection) {
                    eventsSection.scrollIntoView({
                        behavior: 'smooth'
                    });
                }
                
                showToast(`Showing ${sport} events`, 'success');
            }
        });
    });
}

// Event actions
function viewEvent(eventId) {
    const event = currentEvents.find(e => e.id === eventId);
    if (event) {
        showToast(`Viewing ${event.title} details...`, 'success');
        // In a real app, this would navigate to event details page
        console.log('Viewing event:', event);
    }
}

function bookEvent(eventId) {
    const event = currentEvents.find(e => e.id === eventId);
    if (event) {
        showToast(`Booking ${event.title}...`, 'success');
        // In a real app, this would open booking flow
        console.log('Booking event:', event);
    }
}

function loadMoreEvents() {
    // In a real app, this would fetch more events from the API with pagination
    showToast('Loading more events...', 'info', fetchEventsFromAPI);
}

// Toast notifications
function showToast(message, type = 'success', retryCallback = null) {
    if (!toastWrapper) {
        console.error('Toast wrapper not found');
        return;
    }
    
    const toast = document.createElement('div');
    toast.className = `toast ${type}`;
    toast.innerHTML = `
        <div class="toast-icon">
            <i class="fas ${type === 'success' ? 'fa-check-circle' : type === 'info' ? 'fa-info-circle' : 'fa-exclamation-circle'}"></i>
        </div>
        <div class="toast-message">${message}</div>
        ${retryCallback ? '<button class="toast-retry" onclick="retryCallback()">Retry</button>' : ''}
        <button class="toast-close" onclick="removeToast(this)">
            <i class="fas fa-times"></i>
        </button>
    `;
    
    toastWrapper.appendChild(toast);
    
    // Show toast
    setTimeout(() => toast.classList.add('show'), 100);
    
    // Auto remove after 5 seconds
    setTimeout(() => {
        if (toast.parentNode) {
            removeToast(toast.querySelector('.toast-close'));
        }
    }, 5000);
}

function removeToast(closeButton) {
    const toast = closeButton.closest('.toast');
    if (toast) {
        toast.classList.remove('show');
        setTimeout(() => {
            if (toast.parentNode) {
                toast.remove();
            }
        }, 300);
    }
}

// Scroll animations
function initializeScrollAnimations() {
    const observerOptions = {
        threshold: 0.1,
        rootMargin: '0px 0px -50px 0px'
    };

    const observer = new IntersectionObserver((entries) => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                entry.target.classList.add('fade-in-up');
            }
        });
    }, observerOptions);

    // Observe elements for animation
    document.querySelectorAll('.feature-item, .category-item, .event-card').forEach(el => {
        observer.observe(el);
    });
}

// UI updates for logged in user
function updateUIForLoggedInUser(email) {
    const navButtons = document.querySelector('.nav-buttons');
    if (navButtons) {
        navButtons.innerHTML = `
            <div class="user-menu">
                <span class="user-email">${email}</span>
                <button class="btn-outline" onclick="logout()">Logout</button>
            </div>
        `;
    }
}

function logout() {
    showToast('Logged out successfully!', 'success');
    // Reset nav buttons
    const navButtons = document.querySelector('.nav-buttons');
    if (navButtons) {
        navButtons.innerHTML = `
            <button class="btn-outline" onclick="openModal('loginModal')">Login</button>
            <button class="btn-primary" onclick="openModal('registerModal')">Sign Up</button>
            <button class="mobile-menu-btn" id="mobile-menu-btn">
                <i class="fas fa-bars"></i>
            </button>
        `;
        // Reinitialize navigation
        initializeDOMElements();
        initializeNavigation();
    }
}

// Utility functions
function debounce(func, wait) {
    let timeout;
    return function executedFunction(...args) {
        const later = () => {
            clearTimeout(timeout);
            func(...args);
        };
        clearTimeout(timeout);
        timeout = setTimeout(later, wait);
    };
}

function formatDate(dateString) {
    try {
        const date = new Date(dateString);
        const options = { 
            month: 'short', 
            day: 'numeric',
            weekday: 'short'
        };
        return date.toLocaleDateString('en-US', options);
    } catch (error) {
        console.error('Error formatting date:', error);
        return dateString;
    }
}

// Performance optimization
function lazyLoadImages() {
    const images = document.querySelectorAll('img[loading="lazy"]');
    
    if ('IntersectionObserver' in window) {
        const imageObserver = new IntersectionObserver((entries) => {
            entries.forEach(entry => {
                if (entry.isIntersecting) {
                    const img = entry.target;
                    img.src = img.dataset.src || img.src;
                    img.classList.remove('lazy');
                    imageObserver.unobserve(img);
                }
            });
        });

        images.forEach(img => imageObserver.observe(img));
    }
}

// Initialize lazy loading
document.addEventListener('DOMContentLoaded', lazyLoadImages);

// Error handling
window.addEventListener('error', function(e) {
    console.error('JavaScript error:', e.error);
    showToast('Something went wrong. Please try again.', 'error');
});

// Service worker registration (for PWA features)
if ('serviceWorker' in navigator) {
    window.addEventListener('load', function() {
        navigator.serviceWorker.register('/sw.js')
            .then(registration => {
                console.log('SW registered: ', registration);
            })
            .catch(registrationError => {
                console.log('SW registration failed: ', registrationError);
            });
    });
}


// ==== Modal Control ====
function openModal(modalId) {
    const modal = document.getElementById(modalId);
    if (modal) modal.classList.add('active');
}

function closeModal(modalId) {
    const modal = document.getElementById(modalId);
    if (modal) modal.classList.remove('active');
}

function switchModal(currentModalId, targetModalId) {
    closeModal(currentModalId);
    setTimeout(() => openModal(targetModalId), 300);
}

// Close modals when clicking outside them
document.addEventListener('click', function (e) {
    if (e.target.classList.contains('modal-overlay')) {
        e.target.classList.remove('active');
    }
});

// Close modals with ESC key
document.addEventListener('keydown', function (e) {
    if (e.key === 'Escape') {
        document.querySelectorAll('.modal-overlay.active').forEach(modal => {
            modal.classList.remove('active');
        });
    }
});

// ==== Admin Login Logic ====
document.getElementById("login-form").addEventListener("submit", function (e) {
    e.preventDefault();

    const email = document.getElementById("login-email").value.trim();
    const password = document.getElementById("login-password").value.trim();

    // Define admin credentials
    const adminEmail = "mamishovrasul028@gmail.com";
    const adminPassword = "R5561007";

    if (email === adminEmail && password === adminPassword) {
        showToast("Admin login successful!", "success");
        closeModal("loginModal");

        // Optional: Redirect to admin panel
        // window.location.href = "admin.html";
    } else {
        showToast("Invalid email or password", "error");
    }
});

// ==== Toast Notification Function ====
function showToast(message, type = "success") {
    const wrapper = document.getElementById("toast-wrapper");
    if (!wrapper) return;

    const toast = document.createElement("div");
    toast.className = `toast show ${type === "error" ? "error" : ""}`;

    toast.innerHTML = `
        <div class="toast-icon">
            <i class="fas ${type === "error" ? "fa-times-circle" : "fa-check-circle"}"></i>
        </div>
        <div class="toast-message">${message}</div>
        <div class="toast-close" onclick="this.parentElement.remove()">×</div>
    `;

    wrapper.appendChild(toast);

    setTimeout(() => {
        toast.remove();
    }, 4000);
}

// ==== Mobile Menu Toggle ====
const mobileBtn = document.getElementById("mobile-menu-btn");
if (mobileBtn) {
    mobileBtn.addEventListener("click", function () {
        const links = document.getElementById("nav-links");
        if (links) links.classList.toggle("active");
    });
}
