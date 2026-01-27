import { api } from './api';
import type { Attendance } from '@/types';

export const attendanceService = {
  async getAll(): Promise<Attendance[]> {
    return api.get<Attendance[]>('/attendance');
  },

  async getByStudent(studentId: string): Promise<Attendance[]> {
    return api.get<Attendance[]>(`/attendance?studentId=${studentId}`);
  },

  async getByClass(classId: string, date?: string): Promise<Attendance[]> {
    const query = date ? `?date=${date}` : '';
    return api.get<Attendance[]>(`/attendance/class/${classId}${query}`);
  },

  async getByDate(date: string): Promise<Attendance[]> {
    return api.get<Attendance[]>(`/attendance?date=${date}`);
  },

  async markAttendance(attendance: Omit<Attendance, 'id'>): Promise<Attendance> {
    return api.post<Attendance>('/attendance', attendance);
  },

  async bulkMarkAttendance(attendances: Omit<Attendance, 'id'>[]): Promise<Attendance[]> {
    return api.post<Attendance[]>('/attendance/bulk', attendances);
  },

  async update(id: string, attendance: Partial<Attendance>): Promise<Attendance> {
    return api.patch<Attendance>(`/attendance/${id}`, attendance);
  },

  async getStats(classId?: string, startDate?: string, endDate?: string): Promise<{
    present: number;
    absent: number;
    late: number;
    excused: number;
  }> {
    let query = '';
    const params = [];
    if (classId) params.push(`classId=${classId}`);
    if (startDate) params.push(`startDate=${startDate}`);
    if (endDate) params.push(`endDate=${endDate}`);
    if (params.length) query = `?${params.join('&')}`;
    
    return api.get(`/attendance/stats${query}`);
  },
};
