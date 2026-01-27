import { api } from './api';
import type { Grade } from '@/types';

export const gradeService = {
  async getAll(): Promise<Grade[]> {
    return api.get<Grade[]>('/grades');
  },

  async getByStudent(studentId: string): Promise<Grade[]> {
    return api.get<Grade[]>(`/grades?studentId=${studentId}`);
  },

  async getByClass(classId: string): Promise<Grade[]> {
    return api.get<Grade[]>(`/grades/class/${classId}`);
  },

  async create(grade: Omit<Grade, 'id'>): Promise<Grade> {
    return api.post<Grade>('/grades', grade);
  },

  async update(id: string, grade: Partial<Grade>): Promise<Grade> {
    return api.patch<Grade>(`/grades/${id}`, grade);
  },

  async delete(id: string): Promise<void> {
    return api.delete(`/grades/${id}`);
  },

  async getAverageByStudent(studentId: string): Promise<number> {
    const response = await api.get<{ average: number }>(`/grades/average/${studentId}`);
    return response.average;
  },

  async getAverageByClass(classId: string): Promise<number> {
    const response = await api.get<{ average: number }>(`/grades/class/${classId}/average`);
    return response.average;
  },
};
