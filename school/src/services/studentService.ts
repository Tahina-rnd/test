import { api } from './api';
import type { Student } from '@/types';

export const studentService = {
  async getAll(): Promise<Student[]> {
    return api.get<Student[]>('/students');
  },

  async getById(id: string): Promise<Student> {
    return api.get<Student>(`/students/${id}`);
  },

  async getByClass(classId: string): Promise<Student[]> {
    return api.get<Student[]>(`/students?classId=${classId}`);
  },

  async create(student: Omit<Student, 'id'>): Promise<Student> {
    return api.post<Student>('/students', student);
  },

  async update(id: string, student: Partial<Student>): Promise<Student> {
    return api.patch<Student>(`/students/${id}`, student);
  },

  async delete(id: string): Promise<void> {
    return api.delete(`/students/${id}`);
  },

  async assignClass(studentId: string, classId: string): Promise<Student> {
    return api.patch<Student>(`/students/${studentId}/class`, { classId });
  },
};
