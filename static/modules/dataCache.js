// Data Cache Module - Centralized data management
class DataCache {
    constructor() {
        this.cache = {
            dashboard: null,
            employees: null,
            hours: null,
            penalties: null,
            bonuses: null,
            lastUpdated: null
        };
// Mock API URL - in real app, replace with actual API endpoint
        this.apiBaseUrl = 'https://api.example.com/hr';
    }

    async getDashboardData() {
        if (!this.cache.dashboard || this._isCacheExpired()) {
            try {
                // In a real app, this would be a fetch request
                // const response = await fetch(`${this.apiBaseUrl}/dashboard`);
                // this.cache.dashboard = await response.json();
                
                // Mock data
                this.cache.dashboard = {
                    penalties: Math.floor(Math.random() * 10),
                    bonuses: Math.floor(Math.random() * 5),
                    undertime: Math.floor(Math.random() * 20)
                };
                this.cache.lastUpdated = new Date();
            } catch (error) {
                console.error('Failed to fetch dashboard data:', error);
                throw error;
            }
        }
        return this.cache.dashboard;
    }
    async getHoursForEmployee(employeeId) {
        if (!this.cache.hours) {
            this.cache.hours = [];
            this.cache.lastUpdated = new Date();
        }
        
        const hours = this.cache.hours.find(h => h.employeeId === employeeId);
        if (hours) {
            return hours;
        }
        
        // Return default values if not found
        return {
            employeeId,
            regularHours: 0,
            overtime: 0,
            undertime: 0
        };
    }

    async getEmployees() {
if (!this.cache.employees || this._isCacheExpired()) {
            try {
                // In a real app, this would be a fetch request
                // const response = await fetch(`${this.apiBaseUrl}/employees`);
                // this.cache.employees = await response.json();
                
                // Mock data
                this.cache.employees = [
                    {
                        id: 1,
                        fullname: 'John Doe',
                        status: 'hired',
                        salary: 50000,
                        penalties: 2,
                        bonuses: 1
                    },
                    {
                        id: 2,
                        fullname: 'Jane Smith',
                        status: 'hired',
                        salary: 65000,
                        penalties: 0,
                        bonuses: 3
                    },
                    {
                        id: 3,
                        fullname: 'Mike Johnson',
                        status: 'fired',
                        salary: 45000,
                        penalties: 5,
                        bonuses: 0
                    },
                    {
                        id: 4,
                        fullname: 'Sarah Williams',
                        status: 'interview',
                        salary: 55000,
                        penalties: 0,
                        bonuses: 0
                    }
                ];
                this.cache.lastUpdated = new Date();
            } catch (error) {
                console.error('Failed to fetch employees:', error);
                throw error;
            }
        }
        return this.cache.employees;
    }
    async addEmployee(employeeData) {
        try {
            const newEmployee = {
                ...employeeData,
                id: Math.floor(Math.random() * 1000) + 5,
                penalties: 0,
                bonuses: 0
            };
            
            if (this.cache.employees) {
                this.cache.employees.push(newEmployee);
            } else {
                this.cache.employees = [newEmployee];
            }
            
            // Initialize related data
            if (!this.cache.hours) this.cache.hours = [];
            this.cache.hours.push({
                employeeId: newEmployee.id,
                regularHours: 0,
                overtime: 0,
                undertime: 0
            });
            
            if (!this.cache.penalties) this.cache.penalties = [];
            if (!this.cache.bonuses) this.cache.bonuses = [];
            
            return newEmployee;
} catch (error) {
            console.error('Failed to add employee:', error);
            throw error;
        }
    }
    async addHours(employeeId, hoursData) {
        if (!this.cache.hours) this.cache.hours = [];
        
        const existing = this.cache.hours.find(h => h.employeeId === employeeId);
        if (existing) {
            Object.assign(existing, hoursData);
        } else {
            this.cache.hours.push({
                employeeId,
                ...hoursData
            });
        }
        this.cache.lastUpdated = new Date();
    }

    async addPenalty(employeeId, penaltyData) {
        if (!this.cache.penalties) this.cache.penalties = [];
        
        const penalty = {
            id: Date.now(),
            employeeId,
            ...penaltyData,
            date: new Date().toISOString()
        };
        
        this.cache.penalties.push(penalty);
        this.cache.lastUpdated = new Date();
        
        // Update employee's penalty count
        const employee = this.cache.employees?.find(e => e.id === employeeId);
        if (employee) {
            employee.penalties = (employee.penalties || 0) + 1;
        }
    }

    async addBonus(employeeId, bonusData) {
        if (!this.cache.bonuses) this.cache.bonuses = [];
        
        const bonus = {
            id: Date.now(),
            employeeId,
            ...bonusData,
            date: new Date().toISOString()
        };
        
        this.cache.bonuses.push(bonus);
        this.cache.lastUpdated = new Date();
        
        // Update employee's bonus count
        const employee = this.cache.employees?.find(e => e.id === employeeId);
        if (employee) {
            employee.bonuses = (employee.bonuses || 0) + 1;
        }
    }

    _isCacheExpired() {
        if (!this.cache.lastUpdated) return true;
        const now = new Date();
        const diffInMinutes = (now - this.cache.lastUpdated) / (1000 * 60);
        return diffInMinutes > 5;
    }

    clearCache() {
        this.cache = {
            dashboard: null,
            employees: null,
            hours: null,
            penalties: null,
            bonuses: null,
            lastUpdated: null
        };
}
}

// Initialize and expose the data cache
if (!window.dataCache) {
    window.dataCache = new DataCache();
}