// Student types
export interface Student {
  id: string;
  firstName: string;
  lastName: string;
  email: string;
  dateOfBirth: string;
  gender: 'male' | 'female' | 'other';
  classId: string;
  className?: string;
  enrollmentDate: string;
  parentName: string;
  parentPhone: string;
  address: string;
  status: 'active' | 'inactive';
}

// Class types
export interface Class {
  id: string;
  name: string;
  level: 'primary' | 'secondary' | 'high';
  teacherId?: string;
  teacherName?: string;
  studentCount: number;
}

// Grade types
export interface Grade {
  id: string;
  studentId: string;
  studentName?: string;
  subject: string;
  score: number;
  maxScore: number;
  term: string;
  date: string;
  comments?: string;
}

// Attendance types
export interface Attendance {
  id: string;
  studentId: string;
  studentName?: string;
  date: string;
  status: 'present' | 'absent' | 'late' | 'excused';
  notes?: string;
}

// Fee types
export interface Fee {
  id: string;
  studentId: string;
  studentName?: string;
  description: string;
  amount: number;
  dueDate: string;
  status: 'paid' | 'partial' | 'unpaid';
  paidAmount: number;
  paymentDate?: string;
}

export interface Payment {
  id: string;
  feeId: string;
  amount: number;
  date: string;
  method: 'cash' | 'card' | 'transfer' | 'check';
  reference?: string;
}

// Dashboard stats
export interface DashboardStats {
  totalStudents: number;
  totalClasses: number;
  unpaidFees: number;
  unpaidFeesAmount: number;
  attendanceToday: {
    present: number;
    absent: number;
    total: number;
  };
}

// Auth types
export interface User {
  id: string;
  email: string;
  name: string;
  role: 'admin' | 'teacher' | 'staff';
}

export interface LoginCredentials {
  email: string;
  password: string;
}

export interface AuthResponse {
  user: User;
  token: string;
}
